#include "openfhe.h"
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/bfvrns/bfvrns-ser.h"
#include "scheme/bfvrns/cryptocontext-bfvrns.h"
#include "scheme/bfvrns/gen-cryptocontext-bfvrns-internal.h"
#include <chrono>
#include "scheme/bfvrns/bfvrns-leveledshe.h"
#include "scheme/bfvrns/bfvrns-cryptoparameters.h"
#include "schemebase/base-scheme.h"
#include "cryptocontext.h"
#include "ciphertext.h"

using namespace lbcrypto;
using namespace std;
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

void printOut(vector<int64_t> x){
  for(size_t i=0 ; i<x.size() ; i++){
    if(x[i]!=0) cout<<"i:"<<i<<", val:"<<x[i]<<", ";
  }
}

auto PartialSum(Ciphertext<DCRTPoly> ctxt, int64_t length, int64_t num, const CryptoContext<DCRTPoly> &cryptoContext){
  int64_t rotNum = length/num;
  int64_t count = log2(num);
  cout<<"rotate "<<count<<" times"<<endl;
  for (int64_t i=0 ; i<count; i++){
    auto ciphertextRot = ctxt;
    int64_t t=pow(rotNum,i+1);
    ciphertextRot = cryptoContext->EvalRotate(ctxt, t);
    ctxt = cryptoContext->EvalAdd(ctxt, ciphertextRot);
  }
  return ctxt;
}

int main() {
  // Step 1: Set CryptoContext
  CCParams<CryptoContextBFVRNS> parameters;
  // parameters.SetPlaintextModulus(65537);
  // parameters.SetMultiplicativeDepth(17);
  // parameters.SetSecurityLevel(HEStd_128_classic);
  parameters.SetPlaintextModulus(65537);
  parameters.SetMultiplicativeDepth(1);
  // parameters.SetScalingModSize(60);
  parameters.SetSecurityLevel(HEStd_128_classic);

  CryptoContext<DCRTPoly> cryptoContext = GenCryptoContext(parameters);
  // Enable features that you wish to use
  cryptoContext->Enable(PKE);
  cryptoContext->Enable(KEYSWITCH);
  cryptoContext->Enable(LEVELEDSHE);

  int64_t slots = cryptoContext->GetEncodingParams()->GetBatchSize();
  // int64_t ptMod = cryptoContext->GetCryptoParameters()->GetPlaintextModulus();
  int64_t L = parameters.GetMultiplicativeDepth();
  // std::cout<< "plaintext modulus = " << ptMod << std::endl;
  std::cout<< "Depth L = " << L << std::endl;
  std::cout << "p = " << cryptoContext->GetCryptoParameters()->GetPlaintextModulus() << std::endl;
  std::cout << "n(slots) = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder() / 2
            << std::endl;
  std::cout << "m = 2*n =" << cryptoContext->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder()
            << std::endl;
  std::cout << "log2 q = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetModulus().GetMSB()
            << std::endl;

  // Step 2: Key Generation

  // Initialize Public Key Containers
  KeyPair<DCRTPoly> keyPair;

  // Generate a public/private key pair
  keyPair = cryptoContext->KeyGen();

  // Generate the relinearization key
  cryptoContext->EvalMultKeyGen(keyPair.secretKey);

  // Generate the rotation evaluation keys
  // vector<int32_t> indexList = {-1,-2};
  // vector<int32_t> indexList;
  // for(int64_t i=0 ; i<log2(slots); i++){
  //   indexList.push_back(pow(2,i));
  // }
  // cryptoContext->EvalRotateKeyGen(keyPair.secretKey, indexList);

  int times = 1000; // set the test times
  std::vector<Ciphertext<DCRTPoly>> ct1,ct2,ct3,ct4,ct5,ct6,ct7,ct8,ct9;// make two vector of ciphertexts
  std::vector<Plaintext> pt1,pt2; // make two vector of plaintexts
  for(int i=0; i < times ; i++){
    int val=generator()%2;
    vector<int64_t> vec1(slots,val);
    Plaintext plaintext  = cryptoContext->MakePackedPlaintext(vec1);
    auto ciphertext = cryptoContext->Encrypt(keyPair.publicKey, plaintext);
    pt1.push_back(plaintext);
    ct1.push_back(ciphertext);
  }
  ct2=ct1; ct3=ct1; ct4=ct1; ct5=ct1; ct6=ct1; ct7=ct1; ct8=ct1; ct9=ct1;
  pt2=pt1;
  // ///////////////// ct with ct
  // // a-b ct1*ct2 => ct3
  auto Mul_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    ct3[i] = cryptoContext->EvalMult(ct1[i], ct2[i]);
  }
  auto Mul_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec =
          Mul_end_time - Mul_begin_time;
  double mul_result = Mul_sec.count();
  std::cout<< mul_result/(times) <<std::endl;

  // a+b ct1+ct2 => ct3
  auto Add_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    ct4[i] = cryptoContext->EvalAdd(ct1[i], ct2[i]);
  }
  auto Add_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec =
          Add_end_time - Add_begin_time;
  double add_result=Add_sec.count();
  std::cout<<add_result/(times)<<std::endl;

  // a-b ct1-ct2 => ct3
  auto Sub_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    cryptoContext->EvalAddInPlace(ct5[i], ct1[i]);
  }
  auto Sub_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Sub_sec =
          Sub_end_time - Sub_begin_time;
  double sub_result = Sub_sec.count();
  std::cout<<sub_result/(times) <<std::endl;

  ///////////////// ct with pt
  // a-b ct1*pt2 => ct3
  auto Mul_begin_time2 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    // ct8[i] = cryptoContext->EvalMult(ct2[i], pt2[i]);
    cryptoContext->GetScheme()->EvalMultInPlace(ct6[i], pt2[i]);
  }
  auto Mul_end_time2 = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec2 =
          Mul_end_time2 - Mul_begin_time2;
  double mul_result2 = Mul_sec2.count();
  std::cout<< mul_result2/(times) <<std::endl;

  // // a+b ct1+pt2 => ct3
  auto Add_begin_time2 = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    // ct6[i] = cryptoContext->EvalAdd(ct9[i], pt3[i]);
    cryptoContext->GetScheme()->EvalAddInPlace(ct7[i], pt2[i]);
  }
  auto Add_end_time2 = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec2 =
          Add_end_time2 - Add_begin_time2;
  double add_result2 = Add_sec2.count();
  std::cout<<add_result2/(times) <<std::endl;

  auto Mul_begin_time3 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    ct8[i] = cryptoContext->EvalMult(ct1[i], pt2[i]);
    // cryptoContext->GetScheme()->EvalMultInPlace(ct7[i], pt2[i]);
  }
  auto Mul_end_time3 = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec3 =
          Mul_end_time3 - Mul_begin_time3;
  double mul_result3 = Mul_sec3.count();
  std::cout<< mul_result3/(times) <<std::endl;

  // // a+b ct1+pt2 => ct3
  auto Add_begin_time3 = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    ct9[i] = cryptoContext->EvalAdd(ct1[i], pt2[i]);
    // cryptoContext->GetScheme()->EvalAddInPlace(ct8[i], pt3[i]);
  }
  auto Add_end_time3 = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec3 =
          Add_end_time3 - Add_begin_time3;
  double add_result3 = Add_sec3.count();
  std::cout<<add_result3/(times) <<std::endl;

  return 0;
}
