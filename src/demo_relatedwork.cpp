/* This is a demo for related work [MMN22] */
# include "openfhe.h"
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
#define vSize pow(2,6) // <= you need to change the bit length d

using namespace lbcrypto;
using namespace std;

auto EvalSum(Ciphertext<DCRTPoly> ctxt, int64_t length, const CryptoContext<DCRTPoly> &cryptoContext){
    int64_t logN=log2(length);
    auto c_automorph = cryptoContext->EvalAutomorphism(ctxt, length+1, cryptoContext->GetEvalAutomorphismKeyMap(ctxt->GetKeyTag()));
    c_automorph = cryptoContext->EvalAdd(ctxt ,c_automorph);
    for (int64_t k=1 ; k != logN ; ++k){
      auto ciphertextRot = c_automorph;
      int64_t t = length/pow(2,k)+1;
      ciphertextRot = cryptoContext->EvalAutomorphism(c_automorph, t, cryptoContext->GetEvalAutomorphismKeyMap(c_automorph->GetKeyTag()));;
      c_automorph = cryptoContext->EvalAdd(c_automorph, ciphertextRot);
    }
    return c_automorph;
}

// auto TotalSum(Ciphertext<DCRTPoly> ctxt, int64_t length, const CryptoContext<DCRTPoly> &cryptoContext){
//     for (int64_t i=0 ; i<log2(length); i++){
//       auto ciphertextRot = ctxt;
//       int64_t t=pow(2,i);
//       ciphertextRot = cryptoContext->EvalRotate(ctxt, t);
//       ctxt = cryptoContext->EvalAdd(ctxt, ciphertextRot);
//     }
//     return ctxt;
// }

vector<int64_t> read_vector(const string &filename){
  int64_t temp;
	string lineStr;
	vector<int64_t> vec;

  ifstream inFile(filename, ios::in);
	getline(inFile, lineStr);
  stringstream ss(lineStr);
  string str;
	while (getline(ss, str, ' ')){
    temp = std::stoi(str);
		vec.push_back(temp);
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
    parameters.SetMultiplicativeDepth(17);
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

    // Generate the rotation evaluation keys
    // vector<int32_t> indexList = {1,2};
    vector<usint> indexList;
    indexList.push_back(slots+1);
    for(int64_t i=1 ; i != log2(slots); ++i){
      indexList.push_back(slots/pow(2,i)+1);
      cout<<"index:"<<slots/pow(2,i)+1<<endl;
    }
    // indexList.push_back(2 * slots - 1);
    // cryptoContext->EvalAutomorphismKeyGen(keyPair.secretKey, indexList);
    cryptoContext->InsertEvalAutomorphismKey(cryptoContext->EvalAutomorphismKeyGen(keyPair.secretKey, indexList));


    vector<int64_t> vectorOfInLUT = read_vector("Table/RelatedWork/relatedwork_in_6.txt"); // <= you need to change the bit length d
    Plaintext ptOfInLUT= cryptoContext->MakePackedPlaintext(vectorOfInLUT);

    int64_t input_int0 = 2;
    std::vector<int64_t> vectorOfInts0(vSize,input_int0);
    Plaintext plaintextInts0 = cryptoContext->MakePackedPlaintext(vectorOfInts0);
    auto ciphertextInts0 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts0);

    int64_t input_int1 = 1;
    std::vector<int64_t> vectorOfInts1(vSize,input_int1);
    Plaintext plaintextInts1 = cryptoContext->MakePackedPlaintext(vectorOfInts1);
    auto ciphertextInts1 = cryptoContext->Encrypt(keyPair.publicKey, plaintextInts1);

    vector<vector<int64_t>> coeff = read_table("Table/RelatedWork/relatedwork_coeff_6.txt"); // <= you need to change the bit length d
    vector<Plaintext> coeff_pt;
    for(size_t i=0 ; i<coeff.size() ; i++){
        Plaintext temp_pt = cryptoContext->MakePackedPlaintext(coeff[i]);
        coeff_pt.push_back(temp_pt);
    }
    cout<<"Coeff # of row="<<coeff_pt.size()<<"(ptMod="<<ptMod<<")"<<endl;

    std::vector<int64_t> vectorOfZero(vSize,0);
    Plaintext plaintextZero = cryptoContext->MakePackedPlaintext(vectorOfZero);
    std::cout << "plaintextZero: " << plaintextZero << std::endl;
    auto ciphertextZero = cryptoContext->Encrypt(keyPair.publicKey, plaintextZero);

    std::vector<int64_t> vectorOfOne(vSize,1);
    Plaintext plaintextOne = cryptoContext->MakePackedPlaintext(vectorOfOne);
    auto ciphertextOne = cryptoContext->Encrypt(keyPair.publicKey, plaintextOne);

    cout<<"Finish to set pre-computed items"<<endl;

    vector<Ciphertext<DCRTPoly>> ctPowA, ctPowB;// ctPowA: Pow(input, p), ctPowB: Pow(input^p, s)
    int64_t p = sqrt(vSize); // here, N=2^l=vSize here can only accept p as an integer which means p^2=vSize
    int64_t k = log2(p);
    cout<<"p=s="<<p<<", k="<<k<<endl;

    for(int64_t i=0 ; i<=p ; i++){
      Ciphertext<DCRTPoly> temp;
      ctPowA.push_back(temp);
      ctPowB.push_back(temp);
    }

    auto startWhole=chrono::high_resolution_clock::now();
// /* For get the [a,a^2,...,a^{p-1}] results
    cout<<"Compute ctPowA: Pow(input, p)."<<endl;
    ctPowA[0] = ciphertextOne;
    ctPowA[1] = ciphertextInts0;
    cout<<"pow1"<<endl;
    for(int64_t i=0 ; i<k ; i++){
      // cout<<"No.i"<<i<<" ";
      for(int64_t j=1 ; j<=pow(2,i) ; j++){
        int64_t tep = pow(2,i)+j;
        cout<<"No."<<tep<<" ";
        ctPowA[tep] = cryptoContext->EvalMult(ctPowA[(pow(2,i))], ctPowA[j]);
      }
    }
    if(pow(2,k) < p-1){
      for(int64_t i=1 ; i<=(p-1-pow(2,k)) ; i++){
        int64_t tep = pow(2,k)+i;
        ctPowA[tep] = cryptoContext->EvalMult(ctPowA[(pow(2,k))], ctPowA[i]);
      }
    }
// /* For get the [a^p,a^2p,...,a^{sp}] results
    cout<<"Compute ctPowB: Pow(input^p, s)."<<endl;
    ctPowB[0] = ciphertextOne;
    ctPowB[1] = ctPowA[k-1];
    for(int64_t i=0 ; i<k ; i++){
      for(int64_t j=1 ; j<=pow(2,i) ; j++){
        int64_t tep = pow(2,i)+j;
        cout<<"No."<<tep<<" ";
        ctPowB[tep] = cryptoContext->EvalMult(ctPowB[(pow(2,i))], ctPowB[j]);
      }
    }
    if(pow(2,k) < p-1){
      for(int64_t i=1 ; i<=(p-1-pow(2,k)) ; i++){
        int64_t tep = pow(2,k)+i;
        ctPowB[tep] = cryptoContext->EvalMult(ctPowB[(pow(2,k))], ctPowB[i]);
      }
    }
    cout<<"Compute S and F"<<endl;
    Ciphertext<DCRTPoly> ctS, ctF;
    ctS = ciphertextZero;
    for(int64_t i=0 ; i<p ; i++){
      ctF = ciphertextZero;
      for(int64_t j=0 ; j<p ; j++){
        cout<<"No."<<i*p+j<<" ";
        auto temp = cryptoContext->EvalMult(ctPowA[j],coeff_pt[i*p+j]);
        ctF = cryptoContext->EvalAdd(ctF,temp);
      }
      auto tep = cryptoContext->EvalMult(ctF,ctPowB[i]);
      ctS = cryptoContext->EvalAdd(ctS,tep);
    }
// */
    cout<<"Compute One-Hot slot"<<endl;
    auto ciphertextSub = cryptoContext->EvalSub(ptOfInLUT,ciphertextInts1);
    auto ctPow = ciphertextSub;
    int64_t d = log2(ptMod);
    cout<<"depth="<<d<<endl;
    for(int64_t i=0 ; i<d ; i++){
      cout<<"No."<<i<<endl;
      ctPow = cryptoContext->EvalMult(ctPow, ctPow);
    }

    auto ctResult = cryptoContext->EvalSub(plaintextOne, ctPow);
    ctS = cryptoContext->EvalMult(ctS, ctResult);
    // totalSum
    cout<<"EvalSum"<<endl;
    // ctS = EvalSum(ctS,slots,cryptoContext);

    int64_t logN=log2(slots);
    auto c_automorph = cryptoContext->EvalAutomorphism(ctS, slots+1, cryptoContext->GetEvalAutomorphismKeyMap(ctS->GetKeyTag()));
    c_automorph = cryptoContext->EvalAdd(ctS ,c_automorph);
    for (int64_t k=1 ; k != logN ; ++k){
      auto ciphertextRot = c_automorph;
      int64_t t = slots/pow(2,k)+1;
      ciphertextRot = cryptoContext->EvalAutomorphism(c_automorph, t, cryptoContext->GetEvalAutomorphismKeyMap(c_automorph->GetKeyTag()));;
      c_automorph = cryptoContext->EvalAdd(c_automorph, ciphertextRot);
    }

    auto endWhole=chrono::high_resolution_clock::now();

    chrono::duration<double> diffWhole = endWhole-startWhole;
    cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

    // Plaintext pt;
    // cryptoContext->Decrypt(keyPair.secretKey, ctS, &pt);
    // cout<<"The output vector is :"<<pt<<endl;

    return 0;
}
