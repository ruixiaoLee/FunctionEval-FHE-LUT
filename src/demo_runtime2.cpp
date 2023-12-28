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
  parameters.SetPlaintextModulus(65537);
  // parameters.SetMultiplicativeDepth(20);
  parameters.SetScalingModSize(60);
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

  int times = 1000; // set the test times

    // vector<int64_t> vec1={1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0};
    // vector<int64_t> vec2={0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};
    int val1=generator()%2;
    vector<int64_t> vec1(slots,val1);
    int val2=generator()%2;
    vector<int64_t> vec2(slots,val2);
    Plaintext pt1  = cryptoContext->MakePackedPlaintext(vec1);
    auto ct1 = cryptoContext->Encrypt(keyPair.publicKey, pt1);
    Plaintext pt2  = cryptoContext->MakePackedPlaintext(vec2);
    auto ct2 = cryptoContext->Encrypt(keyPair.publicKey, pt2);


  // ///////////////// ct with ct
  // // a-b ct1*ct2 => ct3
  auto Mul_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    auto ct3 = cryptoContext->EvalMult(ct1, ct2);
  }
  auto Mul_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec =
          Mul_end_time - Mul_begin_time;
  double mul_result = Mul_sec.count();
  std::cout<< mul_result/(times) <<std::endl;

  // a+b ct1+ct2 => ct3
  auto Add_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    auto ct4 = cryptoContext->EvalAdd(ct1, ct2);
  }
  auto Add_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec =
          Add_end_time - Add_begin_time;
  double add_result=Add_sec.count();
  std::cout<<add_result/(times)<<std::endl;

  // a-b ct1-ct2 => ct3
  Ciphertext<DCRTPoly> temp = ct1;
  auto Sub_begin_time = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    cryptoContext->EvalAddInPlace(temp, ct2);
  }
  auto Sub_end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> Sub_sec =
          Sub_end_time - Sub_begin_time;
  double sub_result = Sub_sec.count();
  std::cout<<sub_result/(times) <<std::endl;

  ///////////////// ct with pt
  // a-b ct1*pt2 => ct1
  Ciphertext<DCRTPoly> temp2 = ct1;
  auto Mul_begin_time2 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    cryptoContext->GetScheme()->EvalMultInPlace(temp2, pt2);
  }
  auto Mul_end_time2 = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec2 =
          Mul_end_time2 - Mul_begin_time2;
  double mul_result2 = Mul_sec2.count();
  std::cout<< mul_result2/(times) <<std::endl;

  // // a+b ct1+pt2 => ct1
  Ciphertext<DCRTPoly> temp3 = ct1;
  auto Add_begin_time2 = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    cryptoContext->GetScheme()->EvalAddInPlace(temp3, pt2);
  }
  auto Add_end_time2 = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec2 =
          Add_end_time2 - Add_begin_time2;
  double add_result2 = Add_sec2.count();
  std::cout<<add_result2/(times) <<std::endl;

  // a-b ct1*pt2 => ct3
  auto Mul_begin_time3 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    auto ct5 = cryptoContext->EvalMult(ct1, pt2);
  }
  auto Mul_end_time3 = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec3 =
          Mul_end_time3 - Mul_begin_time3;
  double mul_result3 = Mul_sec3.count();
  std::cout<< mul_result3/(times) <<std::endl;

  // // a+b ct1+pt2 => ct3
  auto Add_begin_time3 = chrono::high_resolution_clock::now();
  for(int i=0; i<times;i++){
    auto ct6 = cryptoContext->EvalAdd(ct1, pt2);
  }
  auto Add_end_time3 = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec3 =
          Add_end_time3 - Add_begin_time3;
  double add_result3 = Add_sec3.count();
  std::cout<<add_result3/(times) <<std::endl;

  //////////////
  auto ct9 = cryptoContext->EvalMult(ct1, pt2);
  auto Mul_begin_time11 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    auto tep = cryptoContext->EvalMult(ct9, pt2);
  }
  auto Mul_end_time11 = chrono::high_resolution_clock::now();
  chrono::duration<double> Mul_sec11 =
          Mul_end_time11 - Mul_begin_time11;
  double mul_result11 = Mul_sec11.count();
  std::cout<< "2nd time mul: "<<mul_result11/(times) <<std::endl;
  //////////////
  auto ct11 = cryptoContext->EvalAdd(ct1, pt2);
  auto Add_begin_time12 = chrono::high_resolution_clock::now();
  for(int i=0 ; i<times ; i++){
    auto tep1 = cryptoContext->EvalAdd(ct11, pt2);
  }
  auto Add_end_time12 = chrono::high_resolution_clock::now();
  chrono::duration<double> Add_sec12 =
          Add_end_time12 - Add_begin_time12;
  double add_result12 = Add_sec12.count();
  std::cout<< "2nd time add: "<<add_result12/(times) <<std::endl;

  return 0;
}
