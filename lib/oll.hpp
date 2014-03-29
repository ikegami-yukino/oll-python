#ifndef OLL_HPP__
#define OLL_HPP__

// oll -- online learning library
// Daisuke Okanohara 2008
// 
// References
// ----------------------
// - Perceptron
// [1] F. Rosenblatt, "The Perceptron: A Probabilistic Model for Information Storage and Organization in the Brain", Cornell Aeronautical Laboratory, Psychological Review, v65, No. 6, pp. 386-408, 1958. 
// [2] Y. Freund, and R. E. Schapire, "Large margin classification using the perceptron algorithm". COLT 1998 
//
// - Averaged Perceptron
// [3] M. Collins, "Discriminative Training Methods for Hidden Markov Models: Theory and Experiments with Perceptron Algorithms", EMNLP 2002. 
// 
// - Passive Agressive
// [4] K. Crammer, O. Dekel. J. Keshet, S. Shalev-Shwartz, Y. Singer, "Online Passive-Aggressive Algorithms.", Journal of Machine Learning Research, 2006.
// 
// - Confidence Weighted
// [5] M. Dredze, K. Crammer, F. Pereira. "Confidence-Weighted Linear Classification", ICML 2008
//

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace oll_tool{  
  typedef std::vector<std::pair<int, float> > fv_t; // feature vector
  typedef std::vector<float> fvec;

  enum trainMethod{
    P  = 0,  // Perceptron
    AP  = 1, // Averaged Perceptron
    PA  = 2, // Passive Agressive
    PA1 = 3, // Passive Agressive L1
    PA2 = 4, // Passive Agressive L2
    PAK = 5, // Kernelized Passive Agressive
    CW  = 6, // Confidence Weighted     
    AL  = 7  // ALMA HD
  };
  
  int trainFile(const char* trainfile, const char* modelfile, 
		const trainMethod tm, const float C, const float bias, const int iter, bool verb, bool shuffle);

  int testFile (const char* testfile, const char* modelfile, std::vector<int>& confMat, int verb);

  // For specializing oll::exampleTrain
  struct P_s {};   // Perceptron
  struct AP_s {};  // Averaged Perceptron
  struct PA_s {};  // Passive Agressive
  struct PA1_s {}; // Passive Agressive L1
  struct PA2_s {}; // Passive Agressive L2
  struct PAK_s {}; // Kernelized Passive Agressive
  struct CW_s {};  // Confidence Weighted 
  struct AL_s {};  // ALMA HD

  class oll{
    static const char* trainMethod_s[];

  public:
    oll();
    ~oll();

    template<class T>    
    void trainExample(const T& a, const fv_t& fv, const int y) ;
        
    int save(const char* filename);
    int load(const char* filename);

    float classify(const fv_t& fv);
    float getMargin(const fvec& v, const float bias_, const fv_t& fv) const;
    float getMarginK(const fv_t& fv) ; // kernelized, use margins as buffer
    float getVariance(const fv_t& fv) const;

    float getNorm(const fv_t& fv) const;
    
    template<class T>    
    int trainFile(const T& a, const char* filename, 
		  const int iter = 10,  const bool verb = true, const bool shuffle = true);

    int testFile(const char* filename, std::vector<int>& confMat, const bool verb = false);

    int parseLine(const std::string& line, fv_t& fv, int& y);

    void setC(const float C_);
    void setBias(const float bias_);

    std::string getErrorLog() const;
    std::string getResultLog() const;
    
  private:
    void update(fvec& v, const fv_t& fv, const float alpha);
    void updateCW(const fv_t& fv, const int y, const float alpha);
    void updatePAK(const fv_t& fv, const float alpha);

    void project(fvec& v);
    float inp(const fv_t& fv1, const fv_t& fv2) const;

    template<class T>
    int valWrite(const T& v, FILE* fp, const char* name);

    template<class T>
    int vecWrite(const std::vector<T>& v, FILE* fp, const char* name);

    template<class T>
    int valRead(T& v, FILE* fp, const char* name);

    template<class T>
    int vecRead(std::vector<T>& v, FILE* fp, const char* name);

    size_t exampleN;
    size_t featureN;
    size_t updateN;

    float C;
    float bias;
    
    fvec w;
    float b;     // weight for bias

    // Averaged Perceptron
    fvec w0;  
    float b0;

    fvec wa; 
    float ba;

    // kernelized passive agressive
    fvec alphas;
    std::vector<fv_t> inv_svs; // Inverted File Index for Support Vectors
    fvec margins; // used for getMarginK

    // Confidence Weighted
    fvec cov; 
    float covb; 

    std::ostringstream errorLog;
    std::ostringstream resultLog;
  };

  // Templates

  template<class T>
  int oll::trainFile(const T& a, const char* filename, const int iter, const bool verb, const bool shuffle){
    std::ifstream ifs(filename);
    if (!ifs){
      errorLog << "cannot open " << filename;
      return -1;
    }

    size_t lineN = 0;
    std::string line;
    std::vector<std::pair<fv_t, int> > examples;
    while (getline(ifs, line)){
      lineN++;
      if (line[0] == '#') continue; // comment   

      fv_t fv;
      int  y = 0;
      if (parseLine(line, fv, y) == -1){
	errorLog << "line:" << lineN;
	return -1;
      }

      if (iter == 0){ // on the fly
	trainExample(a, fv, y);
      } else {
	examples.push_back(std::make_pair(fv, y));
      }
    }
    if (verb && iter > 0) std::cout << "Read Done." << std::endl;

    if (shuffle){
      std::random_shuffle(examples.begin(), examples.end());
    }

    for (int i = 0; i < iter; i++){
      for (size_t j = 0; j < examples.size(); j++){
     	trainExample(a, examples[j].first, examples[j].second);
      }
      if (verb) {
	std::cout << ".";
	if ((iter+1) % 50 == 0) std::cout << std::endl;
      }
    }
    if (verb) std::cout << "FINISH!" << std::endl;
    return 0;
  }


  template<class T>
  int oll::valWrite(const T& v, FILE* fp, const char* name){
    if (fwrite(&v, sizeof(T), 1, fp) != 1){ 
     errorLog << "fwrite error " << name;
      return -1;
    }
    return 0;
  }

  template<class T>
  int oll::vecWrite(const std::vector<T>& v, FILE* fp, const char* name){
    size_t n = v.size();
    if (valWrite(n, fp, name) == -1) return -1;
    for (size_t i = 0; i < v.size(); i++){
      T val = v[i];
      if (valWrite(val, fp, name) == -1) return -1;
    }
    return 0;
  }

  template<class T>
  int oll::valRead(T& v, FILE* fp, const char* name){
    if (fread(&v, sizeof(T), 1, fp) != 1){
      errorLog << "fread error " << name;
      return -1;
    }
    return 0;
  }


  template<class T>
  int oll::vecRead(std::vector<T>& v, FILE* fp, const char* name){
    size_t n = 0;
    if (valRead(n, fp, name) == -1) return -1;
    v.clear();
    v.resize(n);

    // Is fread((void*)v.begin(), sizeof(T), n, fp) possible ?
    for (size_t i = 0; i < n; i++){
      T val;
      if (valRead(val, fp, name) == -1) return -1;
      v[i] = val;
    }
    return 0;
  }

}


#endif // OLL_HPP__
