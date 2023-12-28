/* This is a demo for 3-input functions */
#include "openfhe.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/bfvrns/bfvrns-ser.h"
#include "scheme/bfvrns/cryptocontext-bfvrns.h"
#include "scheme/bfvrns/gen-cryptocontext-bfvrns-internal.h"
#include "scheme/cryptocontextparams-base.h"
#include <chrono>
#include "omp.h"
#include <unistd.h>
#include <cstdlib>
// input LUT default 1 row (if not 1 row, the indexList need to be changed)
using namespace lbcrypto;
using namespace std;

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

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

void show_memory_usage(pid_t pid){
  ostringstream path;
  path << "/proc/" << pid << "/status";
  ostringstream cmd;
  cmd << "grep -e 'VmHWM' -e 'VmRSS' -e 'VmSize' -e 'VmStk' -e 'VmData' -e 'VmExe' " << path.str();
  [[maybe_unused]]
  int k = system(cmd.str().c_str());
  return;
}

int main() {
    // Step 1: Set CryptoContext
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(19);
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

    size_t vSize = pow(2,6); // input table row size // <= you need to change the bit length d

    vector<int32_t> indexList;
    int64_t t = slots/vSize;
    for(int64_t i=0 ; i<log2(vSize); i++){
      indexList.push_back(t*pow(2,i));
    }
    cryptoContext->EvalRotateKeyGen(keyPair.secretKey, indexList);
/* set all paramenters */
    // read input/output LUT, input => plaintexts, output => ciphertexts
    vector<vector<int64_t>> vectorOfInLUT = read_table("Table/128bit/three/vectorOfInLUT_three_6.txt"); // <= you need to change the bit length d
    Plaintext plaintextIns = cryptoContext->MakePackedPlaintext(vectorOfInLUT[0]);
    vector<vector<int64_t>> vectorOfOuts = read_table("Table/128bit/three/vectorOfOutLUT_three_6.txt"); // <= you need to change the bit length d
    vector<Plaintext> plaintextOuts;
    for(size_t i=0 ; i<vectorOfOuts.size() ; i++){
      Plaintext temp_pt = cryptoContext->MakePackedPlaintext(vectorOfOuts[i]);
      plaintextOuts.push_back(temp_pt);
    }
    // set input value, ciphertext
    // int64_t input_int0 = generator()% vSize;
    int64_t input_int0 = 1;
    std::vector<int64_t> vectorOfInts0;
    vectorOfInts0 = fillSlot(vectorOfInts0, input_int0, slots, vSize);
    Plaintext plaintextInts0 = cryptoContext->MakePackedPlaintext(vectorOfInts0);
    auto ciphertextInts0 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts0);
    // int64_t input_int1 = generator()% vSize;
    int64_t input_int1 = 1;
    std::vector<int64_t> vectorOfInts1;
    vectorOfInts1 = fillSlot(vectorOfInts1, input_int1, slots, vSize);
    Plaintext plaintextInts1 = cryptoContext->MakePackedPlaintext(vectorOfInts1);
    auto ciphertextInts1 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts1);
    // int64_t input_int2 = generator()% vSize;
    int64_t input_int2 = 1;
    std::vector<int64_t> vectorOfInts2;
    vectorOfInts2 = fillSlot(vectorOfInts2, input_int2, slots, vSize);
    Plaintext plaintextInts2 = cryptoContext->MakePackedPlaintext(vectorOfInts2);
    auto ciphertextInts2 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts2);
    // set all-one, plaintext
    std::vector<int64_t> vectorOfOne;
    vectorOfOne = fillSlot(vectorOfOne, 1, slots, vSize);
    Plaintext plaintextOne = cryptoContext->MakePackedPlaintext(vectorOfOne);

    std::vector<vector<int64_t> > IndexLut = read_table("Table/128bit/three/IndexLut_three_6.txt"); // <= you need to change the bit length d
    vector<Plaintext> IndexLut_pt;
    for(size_t i=0 ; i<IndexLut.size() ; i++){
      IndexLut_pt.push_back(cryptoContext->MakePackedPlaintext(IndexLut[i]));
    }

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
    cryptoContext->GetScheme()->EvalSubInPlace(ciphertextInts2, plaintextIns);

    for(int64_t i=0 ; i<k ; i++){
      ciphertextInts0 = cryptoContext->EvalMult(ciphertextInts0, ciphertextInts0);
      ciphertextInts1 = cryptoContext->EvalMult(ciphertextInts1, ciphertextInts1);
      ciphertextInts2 = cryptoContext->EvalMult(ciphertextInts2, ciphertextInts2);
    }
    ciphertextInts0 = cryptoContext->EvalSub(plaintextOne, ciphertextInts0);
    ciphertextInts1 = cryptoContext->EvalSub(plaintextOne, ciphertextInts1);
    ciphertextInts2 = cryptoContext->EvalSub(plaintextOne, ciphertextInts2);

    #pragma omp parallel for
    for(size_t i=0 ; i<plaintextOuts.size() ; i++){
      cout<<"No."<<i<<endl;
      auto temp0 = cryptoContext->EvalMult(ciphertextInts0, IndexLut_pt[i/vSize]);
      temp0 = PartialSum(temp0,slots,vSize,cryptoContext);
      auto temp1 = cryptoContext->EvalMult(ciphertextInts1, IndexLut_pt[i%vSize]);
      temp1 = PartialSum(temp1,slots,vSize,cryptoContext);
      temp0 = cryptoContext->EvalMult(temp0, temp1);
      temp0 = cryptoContext->EvalMult(temp0, ciphertextInts2);
      cryptoContext->GetScheme()->EvalMultInPlace(temp0, plaintextOuts[i]);
      result[i]=temp0;
    }

    // totalSum
    auto ctResult=result[0];
    for(size_t i=1 ; i<result.size() ; i++){
      cryptoContext->EvalAddInPlace(ctResult, result[i]);
    }
    ctResult = PartialSum(ctResult,slots,vSize,cryptoContext);

    auto endWhole=chrono::high_resolution_clock::now();

    // Plaintext pt;
    // cryptoContext->Decrypt(keyPair.secretKey, ctResult, &pt);
    // cout<<"The output vector is :"<<pt<<endl;

    // std::cout << "Ciphertext ctResult:" << std::endl;
    // std::cout << "scaling factor: " << ctResult->GetScalingFactor() << std::endl;
    // std::cout << "scaling factor degree: " << ctResult->GetNoiseScaleDeg() << std::endl;
    // std::cout << "level: " << ctResult->GetLevel() << std::endl;

    chrono::duration<double> diffWhole = endWhole-startWhole;
    cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;
    show_memory_usage(getpid());

    return 0;
}
