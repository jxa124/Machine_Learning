#ifndef SAMPLE_H
#define SAMPLE_H

#include <iosfwd>
#include <vector>
#include <list>
#include <string>

namespace ID3 {

	//代表一行样本数据
	struct Sample
	{
		typedef std::size_t size_t;

		size_t headache;
		size_t courbature;
		size_t temperature;
		size_t birdFlu;

		size_t getAttrValue(const std::string& attrName) const
		{
			if (attrName == "headache")
				return headache;
			else if (attrName == "courbature")
				return courbature;
			else if (attrName == "temperature")
				return temperature;
			return 0;
		}

		size_t getDecision() const { return birdFlu; }
	}; 
	//代表一个属性
	typedef std::string Attribute;
	//样本集类型
	typedef std::list<Sample> Samples;
	//属性集类型
	typedef std::list<Attribute> Attributes;

	std::istream& operator>>(std::istream& is, Sample& sample);

	void CollectSamples(Samples& samples, Attributes& attributes, const std::string& filename);

	Samples GenerateSamplesByAttrValue(const Samples& samples, const Attribute& attr, std::size_t attrValue);

	void PrintSamples(std::ostream& os, const Attributes& attributes, const Samples& samples);
}
#endif // SAMPLE_H
