#include "Sample.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <cstdlib>
using namespace std;

namespace ID3 {


	
	//重载运算符 输入运算符 >> ：输入一个sample
	std::istream& operator>>(std::istream& is, Sample& sample)
	{
		is >> sample.headache >> sample.courbature >> sample.temperature >> sample.birdFlu;
		return is;
	}

	// 生成 属性集 attributes_list+数据集 samples
	void CollectSamples(Samples& samples, Attributes& attributes, const std::string& filename)
	{
		ifstream sampleStream(filename);
		if (!sampleStream) {
			cerr << "cannot open file: " + filename;
			exit(EXIT_FAILURE);
		}
		// 生成 attributes_list
		string attrLine;
		getline(sampleStream, attrLine); //读取第一行
		stringstream ss(attrLine);
		Attribute attribute;
		while (ss >> attribute) {
			attributes.push_back(attribute);
		}
		// 生成 samples
		Sample sample;
		while (sampleStream >> sample) {
			samples.push_back(sample);
		}
	}


	Samples GenerateSamplesByAttrValue(const Samples& samples, const Attribute& attr, std::size_t attrValue)
	{
		Samples newSamples;
		Samples::const_iterator last = samples.end();
		for (auto first = samples.begin(); first != last; ++first) {
			if (first->getAttrValue(attr) == attrValue) {
				newSamples.push_back(*first);
			}
		}
		return newSamples;
	}

	void PrintSamples(std::ostream& os, const Attributes& attributes, const Samples& samples)
	{
		copy(attributes.begin(), attributes.end(),
			ostream_iterator<string>(os, "\t"));
		os << endl;
		Samples::const_iterator last = samples.end();
		for (auto first = samples.begin(); first != last; ++first) {
			os << first->headache << "\t\t"
				<< first->courbature << "\t\t"
				<< first->temperature << "\t\t"
				<< first->birdFlu << endl;
		}
	}
}