#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath> // sqrt
#include "oll.hpp"

namespace oll_tool{  
  int trainFile(const char* trainfile, const char* modelfile, const trainMethod tm, 
		const float C, const float bias, const int iter, bool verb, bool shuffle){
    oll ol;
    ol.setC(C);
    ol.setBias(bias);
    
    bool error = false;
    if (tm == P){
      P_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == AP){
      AP_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == PA){
      PA_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == PA1){
      PA1_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == PA2){
      PA2_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == PAK){
      PAK_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == CW){
      CW_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else if (tm == AL){
      AL_s a;
      if (ol.trainFile(a, trainfile, iter, verb, shuffle) == -1) error = true;
    } else {
      if (verb){
	std::cerr << "unknown trainMethod" << std::endl;
      }
      return -1;
    }

    if (error){
      if (verb){
	std::cerr << ol.getErrorLog() << std::endl;
      }
      return -1;
    }

    if (ol.save(modelfile) == -1){
      if (verb){
	std::cerr << ol.getErrorLog() << std::endl;
      }
      return -1;
    }
    return 0;
  }

  int testFile (const char* testfile, const char* modelfile, std::vector<int>& confMat, int verb){
    oll ol;
    if (ol.load(modelfile) == -1){
      if (verb){
	std::cerr << ol.getErrorLog() << std::endl;
      }
      return -1;
    }

    bool verb2 = false;
    if (verb == 2) verb2 = true;
    
    if (ol.testFile(testfile, confMat, verb2) == -1){
      if (verb){
	std::cerr << ol.getErrorLog() << std::endl;
      }
      return -1;
    }

    if (verb2){
      std::cout << ol.getResultLog();
    }

    if (verb >= 1){
      const int correctNum = confMat[0] + confMat[3];
      const int num        = confMat[0] + confMat[1] + confMat[2] + confMat[3];
      printf("Accuracy %.3f%% (%d/%d)\n"
	     "(Answer, Predict): (p,p):%d (p,n):%d (n,p):%d (n,n):%d\n", (float)correctNum*100.f/num, correctNum, num, 
	     confMat[0], confMat[1], confMat[2], confMat[3]);
    }

    return 0;
  }

  int oll::parseLine(const std::string& line, fv_t& fv, int& y){
    std::istringstream is(line);
    if (!(is >> y)){
      errorLog << "parse error: no label ";
      return -1;
    }
    
    if (y != 1 &&
	y != -1){
      errorLog << "parse error: y is not +1 nor -1 ";
    }
    
    int  id = 0;
    char sep = 0;
    float val = 0.f;
    while (is >> id >> sep >> val){
      fv.push_back(std::make_pair(id, val));
    }
    return 0;
  }


  oll::oll() : exampleN(0), featureN(0), updateN(0), C(1.f), bias(0.f), b(0.f), b0(0.f), ba(0.f), covb(0.f) {}
  oll::~oll() {}

  void oll::setC(const float C_){
    C = C_;
  }
  
  void oll::setBias(const float bias_){
    bias = bias_;
  }

  float oll::getMargin(const fvec& v, const float bias_, const fv_t& fv) const {
    float ret = bias_;
    for (size_t i = 0; i < fv.size(); i++){
      if (v.size() <= fv[i].first) continue;
      ret += v[fv[i].first] * fv[i].second;
    }
    return ret;
  }

  float oll::getMarginK(const fv_t& fv) { // kernel
    for (size_t i = 0; i < margins.size(); i++){
      margins[i] = 0.f;
    }

    for (size_t i = 0; i < fv.size(); i++){
      const int id = fv[i].first;
      const float val = fv[i].second;
      if (id >= inv_svs.size()) continue;
      const fv_t& ifv = inv_svs[id];
      for (size_t j = 0; j < ifv.size(); j++){
	margins[ifv[j].first] += ifv[j].second * val;
      }
    }

    float ret = 0.f;
    for (size_t i = 0; i < margins.size(); i++){
      ret += (margins[i] * margins[i]) * alphas[i]; // 2nd polynomial
    }
    return ret;
  }

  float oll::getVariance(const fv_t& fv) const{
    float ret = 0.f;
    for (size_t i = 0; i < fv.size(); i++){
      if (cov.size() <= fv[i].first){
	ret += 1.f * fv[i].second * fv[i].second; // assume cov[fv[i].first=1
      } else {
	ret += cov[fv[i].first] * fv[i].second * fv[i].second;
      }
    }
    return ret;
  }

  float oll::getNorm(const fv_t& fv) const{
    float ret = 1.f; // bias
    for (size_t i = 0; i < fv.size(); i++){
      ret += fv[i].second * fv[i].second;
    }
    return ret;
  }

  void oll::update(fvec& v, const fv_t& fv, const float alpha) {
    for (size_t i = 0; i < fv.size(); i++){
      if (v.size() <= fv[i].first) v.resize(fv[i].first+1);
      v[fv[i].first] += fv[i].second * alpha;
    }
    b += alpha * bias;
    updateN++;
  }


  // perceptron
  template <>
  void oll::trainExample(const P_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    printf("score:%f\n", score);
    if (score <= 0.f){
      update(w, fv, y);
    }
    exampleN++;
  }
  
  // averaged perceptron
  template <>
  void oll::trainExample(const AP_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w0, b0, fv) * y;
    if (score <= 0.f){
      update(w0, fv, y);
      update(wa, fv, y * (exampleN+1.f));
    }
    exampleN++;
  }

  // passive agressive
  template <>
  void oll::trainExample(const PA_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    if (score <= 1.f){
      update(w, fv, y * (1.f - score) / getNorm(fv));
    }
    exampleN++;
  }

  // passive agressive-I
  template <>
  void oll::trainExample(const PA1_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    if (score <= 1.f){      
      update(w, fv, y * std::min(C, (1.f - score) / getNorm(fv)));
    }
    exampleN++;    
  }

  // passive agressive-II
  template <>
  void oll::trainExample(const PA2_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    if (score <= 1.f){
      update(w, fv, y * (1.f - score) / (getNorm(fv) + 1 / 2.f / C));
    }
    exampleN++;    
  }

  void oll::updatePAK(const fv_t& fv, const float alpha){
    int svs_id = (int)alphas.size();
    for (size_t i = 0; i < fv.size(); i++){
      if (inv_svs.size() <= fv[i].first) inv_svs.resize(fv[i].first+1);
      inv_svs[fv[i].first].push_back(std::make_pair(svs_id, fv[i].second));
    }
    alphas.push_back(alpha);
    margins.push_back(0.f);
  }

  // kernelized passive agressive 
  template <>
  void oll::trainExample(const PAK_s& a, const fv_t& fv, const int y) {
    const float score = getMarginK(fv) * y;
    if (score <= 1.f){
      updatePAK(fv, y * (1.f - score) / getNorm(fv));
    }
    exampleN++;
  }

  // Confidence-Weighted 
  void oll::updateCW(const fv_t& fv, const int y, const float alpha) {
    for (size_t i = 0; i < fv.size(); i++){
      if (cov.size() <= fv[i].first) {
	w.resize(fv[i].first+1);
	const size_t prevSize = cov.size();
	cov.resize(fv[i].first+1);
	for (size_t j = prevSize; j < cov.size(); j++){
	  cov[j] = 1.f;
	}
      }
      w[fv[i].first] += fv[i].second * alpha * y * cov[fv[i].first];
      cov[fv[i].first] = 1.f / (1.f/cov[fv[i].first] + 2.f * alpha * C * fv[i].second * fv[i].second);
    }
    b += alpha * y * covb * bias;
    covb = 1.f / (1.f/covb + 2.f * alpha * C * bias * bias);
  }

  template <>
  void oll::trainExample(const CW_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    const float var   = getVariance(fv);
    
    const float b     = 1.f+2*C*score;
    const float gamma = (-b + sqrt(b*b-8*C*(score-C*var))) / (4.f * C * var);

    if (gamma > 0){
      updateCW(fv, y, gamma);
    }
  }

  // ALMA HD
  template<>
  void oll::trainExample(const AL_s& a, const fv_t& fv, const int y) {
    const float score = getMargin(w, b, fv) * y;
    if (score <= 0.f){
      update(w, fv, y * sqrt(2.f / getNorm(fv) / (updateN+1)));
      project(w); // too heavy..
    }
    exampleN++;
  }

  void oll::project(fvec& v){
    float sum = 0.f;
    for (size_t i = 0; i < v.size(); i++){
      sum += v[i] * v[i];
    }
    if (sum < 1.f) return;
    const float sum_sqroot = sqrt(sum);
    for (size_t i = 0; i < v.size(); i++){
      v[i] / sum_sqroot;
    }
  }


  int oll::save(const char* filename){
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL){
      errorLog << "Unable to open " << filename;
      return -1;
    }

    if (valWrite(exampleN, fp, "exampleN") == -1) { fclose(fp); return -1;}
    if (valWrite(featureN, fp, "featureN") == -1) { fclose(fp); return -1;}
    if (valWrite(updateN , fp, "updateN" ) == -1) { fclose(fp); return -1;}
    if (valWrite(C,        fp, "C"       ) == -1) { fclose(fp); return -1;}
    if (valWrite(bias,     fp, "bias"    ) == -1) { fclose(fp); return -1;}
    if (vecWrite(w,        fp, "w"       ) == -1) { fclose(fp); return -1;}
    if (valWrite(b,        fp, "b"       ) == -1) { fclose(fp); return -1;}
    if (vecWrite(w0,       fp, "w0"      ) == -1) { fclose(fp); return -1;}
    if (valWrite(b0,       fp, "b0"      ) == -1) { fclose(fp); return -1;}
    if (vecWrite(wa,       fp, "wa"      ) == -1) { fclose(fp); return -1;}
    if (valWrite(ba,       fp, "ba"      ) == -1) { fclose(fp); return -1;}
    if (vecWrite(cov,      fp, "cov"     ) == -1) { fclose(fp); return -1;}
    if (valWrite(covb,     fp, "covb"    ) == -1) { fclose(fp); return -1;}
    if (vecWrite(alphas,   fp, "alphas"  ) == -1) { fclose(fp); return -1;}
    if (vecWrite(inv_svs,  fp, "inv_svs" ) == -1) { fclose(fp); return -1;}

    fclose(fp);

    return 0;
  }

  int oll::load(const char* filename){
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL){
      errorLog << "Unable to open " << filename;
      return -1;
    }

    if (valRead(exampleN, fp, "exampleN") == -1) { fclose(fp); return -1;}
    if (valRead(featureN, fp, "featureN") == -1) { fclose(fp); return -1;}
    if (valRead(updateN,  fp, "updateN" ) == -1) { fclose(fp); return -1;}
    if (valRead(C,        fp, "C"       ) == -1) { fclose(fp); return -1;}
    if (valRead(bias,     fp, "bias"    ) == -1) { fclose(fp); return -1;}
    if (vecRead(w,        fp, "w"       ) == -1) { fclose(fp); return -1;}
    if (valRead(b,        fp, "b"       ) == -1) { fclose(fp); return -1;}
    if (vecRead(w0,       fp, "w0"      ) == -1) { fclose(fp); return -1;}
    if (valRead(b0,       fp, "b0"      ) == -1) { fclose(fp); return -1;}
    if (vecRead(wa,       fp, "wa"      ) == -1) { fclose(fp); return -1;}
    if (valRead(ba,       fp, "ba"      ) == -1) { fclose(fp); return -1;}
    if (vecRead(cov,      fp, "cov"     ) == -1) { fclose(fp); return -1;}
    if (valRead(covb,     fp, "covb"    ) == -1) { fclose(fp); return -1;}
    if (vecRead(alphas,   fp, "alphas"  ) == -1) { fclose(fp); return -1;}
    if (vecRead(inv_svs,  fp, "inv_svs" ) == -1) { fclose(fp); return -1;}
    margins.resize(alphas.size()); // buffer
    fclose(fp);
    return 0;
  }

  float oll::classify(const fv_t& fv) {
    if (w.size() > 0){ // except AP, PAK
      return getMargin(w, b, fv);
    } else if (w0.size() > 0){ // AP
      return getMargin(w0, b0, fv) - getMargin(wa, ba, fv) / (exampleN+1);
    } else { // PAK
      return getMarginK(fv);
    }
  }

  int oll::testFile(const char* filename, std::vector<int>& confMat, const bool verb){
    std::ifstream ifs(filename);
    if (!ifs){
      errorLog << "cannot open " << filename;
      return -1;
    }

    std::string line;
    size_t lineN = 0;

    confMat.clear();
    confMat.resize(4); // pp, pn, np, nn
    while (getline(ifs, line)){
      lineN++;
      if (line[0] == '#') continue;
      
      fv_t fv;
      int  y = 0;
      if (parseLine(line, fv, y) == -1){
	errorLog << "line:" << lineN;
	return -1;
      }
      
      const float score = classify(fv);
      if (verb){
	resultLog << score << std::endl;
      }

      if (score >= 0 && y == 1){
	confMat[0]++;
      } else if (score < 0 && y == 1){
	confMat[1]++;
      } else if (score >= 0 && y == -1){
	confMat[2]++;
      } else if (score < 0 && y == -1){
	confMat[3]++;
      } else {
	errorLog << "error score:" << score << " y:" << y << " line:" << lineN;
	return -1;
      }
    }

    return 0;
  }

  std::string oll::getErrorLog() const{
    return errorLog.str();
  }

  std::string oll::getResultLog() const{
    return resultLog.str();
  }


  template<>
  int oll::valRead(fv_t& fv, FILE* fp, const char* name){
    size_t n = 0;
    if (valRead(n, fp, name) == -1) return -1;
    fv.resize(n);
    for (size_t i = 0; i < n; i++){
      int id = 0;
      float val = 0.f;
      if (valRead(id, fp, name) == -1) return -1;
      if (valRead(val, fp, name) == -1) return -1;
      fv[i] = std::make_pair(id, val);
    }
    return 0;
  }
  
  template<>
  int oll::valWrite(const fv_t& fv, FILE* fp, const char* name){
    size_t n = fv.size();
    if (valWrite(n, fp, name) == -1) return -1;
    for (size_t i = 0; i < fv.size(); i++){
      int id = fv[i].first;
      float val = fv[i].second;
      if (valWrite(id, fp, name) == -1) return -1;
      if (valWrite(val, fp, name) == -1) return -1;
    }
    return 0;
  }
}
