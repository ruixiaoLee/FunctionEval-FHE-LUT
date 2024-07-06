/* This is a demo for 2-input functions */
#include "openfhe.h"
#include <chrono>
#include "omp.h"
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
// input LUT default 1 row (if not 1 row, the indexList need to be changed)

using namespace lbcrypto;
using namespace std;
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

void show_memory_usage(pid_t pid){
  ostringstream path;
  path << "/proc/" << pid << "/status";
  ostringstream cmd;
  cmd << "grep -e 'VmHWM' -e 'VmRSS' -e 'VmSize' -e 'VmStk' -e 'VmData' -e 'VmExe' " << path.str();
  [[maybe_unused]]
  int k = system(cmd.str().c_str());
  return;
}

auto PartialSum(Ciphertext<DCRTPoly> ctxt, int64_t length, int64_t num, const CryptoContext<DCRTPoly> &cryptoContext){
  int64_t rotNum = length/num;
  int64_t count = log2(num);
  for (int64_t i=0 ; i<count; i++){
    auto ciphertextRot = ctxt;
    int64_t t = rotNum * pow(2,i);
    ciphertextRot = cryptoContext->EvalRotate(ctxt, t);
    cryptoContext->EvalAddInPlace(ctxt, ciphertextRot);
  }
  return ctxt;
}

vector<int64_t> fillSlot(vector<int64_t> vec, int64_t val, int64_t length, int64_t num){
  int64_t temp = length/num;
  for(int64_t i=0 ; i<length ; i++){
    if(i % temp == 0) vec.push_back(val);
    else vec.push_back(0);
  }
  return vec;
}

vector<vector <int64_t> > read_table(const string &filename){
  int64_t temp;
	string lineStr;
	vector<vector<int64_t> > table;

  ifstream inFile(filename, ios::in);
	while (getline(inFile, lineStr)){
    vector<int64_t> table_col;
		stringstream ss(lineStr);
		string str;
		while (getline(ss, str, ' ')){
      temp = std::stoi(str);
			table_col.push_back(temp);
    }
		table.push_back(table_col);
	}
  return table;
}

int main() {
    // Step 1: Set CryptoContext
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(18);
    // parameters.SetSecurityLevel(HEStd_NotSet);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cryptoContext = GenCryptoContext(parameters);
    // Enable features that you wish to use
    cryptoContext->Enable(PKE);
    cryptoContext->Enable(KEYSWITCH);
    cryptoContext->Enable(LEVELEDSHE);

    int64_t slots = cryptoContext->GetEncodingParams()->GetBatchSize();
    int64_t ptMod = cryptoContext->GetCryptoParameters()->GetPlaintextModulus();
    int64_t L = parameters.GetMultiplicativeDepth();
    std::cout<< "plaintext modulus = " << ptMod << std::endl;
    std::cout<< "Depth L = " << L << std::endl;
    std::cout << "p = " << cryptoContext->GetCryptoParameters()->GetPlaintextModulus() << std::endl;
    // std::cout << "phim = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetRingDimension() << std::endl;
    std::cout << "n(slots) = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder() / 2
              << std::endl;
    std::cout << "m = 2*n =" << cryptoContext->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder()
              << std::endl;
    std::cout << "log2 q = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetModulus().GetMSB()
              << std::endl;

    // Step 2: Key Generation
    KeyPair<DCRTPoly> keyPair;
    keyPair = cryptoContext->KeyGen();
    cryptoContext->EvalMultKeyGen(keyPair.secretKey);

    int64_t vSize=pow(2,2); // input table row size /* change the bit here! */

    vector<int32_t> indexList;
    int64_t t = slots/vSize;
    for(int64_t i=0 ; i<log2(vSize); i++){
      indexList.push_back(t*pow(2,i));
    }
    cryptoContext->EvalRotateKeyGen(keyPair.secretKey, indexList);
/* set all paramenters */
    // read input/output LUT, input => plaintexts, output => ciphertexts
     vector<vector<int64_t>> vectorOfInLUT = read_table("Table/128bit/two/vectorOfInLUT_two_2.txt");/* change the bit here! */
     Plaintext plaintextIns = cryptoContext->MakePackedPlaintext(vectorOfInLUT[0]);

     vector<vector<int64_t>> vectorOfOuts = read_table("Table/128bit/two/vectorOfOutLUT_two_2.txt");/* change the bit here! */
     vector<Plaintext> plaintextOuts;
     for(size_t i=0 ; i<vectorOfOuts.size() ; i++){
         Plaintext temp_pt = cryptoContext->MakePackedPlaintext(vectorOfOuts[i]);
         plaintextOuts.push_back(temp_pt);
     }
     cout<<"vectorOfOuts row:"<<vectorOfOuts.size()<<endl;

     // set input value, ciphertext
     // int64_t input_int0 = generator()% vSize;
     int64_t input_int0 = 2;
     std::vector<int64_t> vectorOfInts0;
     vectorOfInts0 = fillSlot(vectorOfInts0, input_int0, slots, vSize);
     Plaintext plaintextInts0 = cryptoContext->MakePackedPlaintext(vectorOfInts0);
     auto ciphertextInts0 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts0);
     // int64_t input_int1 = generator()% vSize;
     int64_t input_int1 = 3;
     std::vector<int64_t> vectorOfInts1;
     vectorOfInts1 = fillSlot(vectorOfInts1, input_int1, slots, vSize);
     Plaintext plaintextInts1 = cryptoContext->MakePackedPlaintext(vectorOfInts1);
     auto ciphertextInts1 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts1);
     // set all-one, plaintext
     std::vector<int64_t> vectorOfOne;
     vectorOfOne = fillSlot(vectorOfOne, 1, slots, vSize);
     Plaintext plaintextOne = cryptoContext->MakePackedPlaintext(vectorOfOne);

     std::vector<vector<int64_t> > IndexLut = read_table("Table/128bit/two/IndexLut_two_2.txt");
     vector<Plaintext> IndexLut_pt;
     for(size_t i=0 ; i<IndexLut.size() ; i++){
       IndexLut_pt.push_back(cryptoContext->MakePackedPlaintext(IndexLut[i]));
     }
      cout<<"IndexLut row:"<<IndexLut.size()<<endl;

     vector<Ciphertext<DCRTPoly>> result;
     for(size_t i=0 ; i<plaintextOuts.size() ; i++){
        Ciphertext<DCRTPoly> tem;
        result.push_back(tem);
     }

    int64_t k=log2(ptMod-1);
    cout<<"depth="<<k<<endl;

    auto startWhole=chrono::high_resolution_clock::now();

    cryptoContext->GetScheme()->EvalSubInPlace(ciphertextInts0, plaintextIns);
    cryptoContext->GetScheme()->EvalSubInPlace(ciphertextInts1, plaintextIns);

    for(int64_t i=0 ; i<k ; i++){
      // cout<<"No."<<i<<endl;
      ciphertextInts0 = cryptoContext->EvalMult(ciphertextInts0, ciphertextInts0);
      ciphertextInts1 = cryptoContext->EvalMult(ciphertextInts1, ciphertextInts1);
    }

    ciphertextInts0 = cryptoContext->EvalSub(plaintextOne, ciphertextInts0);
    ciphertextInts1 = cryptoContext->EvalSub(plaintextOne, ciphertextInts1);

    #pragma omp parallel for
    for(size_t i=0 ; i<plaintextOuts.size() ; i++){
      cout<<"No."<<i<<endl;
      auto temp = cryptoContext->EvalMult(ciphertextInts0, IndexLut_pt[i]);
      temp = PartialSum(temp,slots,vSize,cryptoContext);
      temp = cryptoContext->EvalMult(temp, ciphertextInts1);
      cryptoContext->GetScheme()->EvalMultInPlace(temp, plaintextOuts[i]);
      result[i]=temp;
    }

    // totalSum
    auto ctResult=result[0];
    cout<<"The result size="<<result.size()<<endl;
    for(size_t i=1 ; i<result.size() ; i++){
      cryptoContext->EvalAddInPlace(ctResult, result[i]);
    }
    ctResult = PartialSum(ctResult,slots,vSize,cryptoContext);

    auto endWhole=chrono::high_resolution_clock::now();

    // Plaintext pt;
    // cryptoContext->Decrypt(keyPair.secretKey, ctResult, &pt);
    // cout<<"The output vector is :"<<pt<<endl;

    chrono::duration<double> diffWhole = endWhole-startWhole;
    cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;
    show_memory_usage(getpid());

    return 0;
}
