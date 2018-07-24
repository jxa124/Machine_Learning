#include "DecisionTree.h"
#include <set>
#include <algorithm>
#include <functional>
#include <utility>
#include <cmath>
#include <iostream>
#include <string>
using namespace std;

namespace {
	inline void PrintSpace(std::ostream& os, std::size_t spaceCnt)
	{
		string spaces(spaceCnt, ' ');
		os << spaces;
	}
}

namespace ID3 {

	void DecisionNode::print(std::ostream& os) const
	{
		os << "(" << attr;
		if (type == DN_LEAF_TYPE) {
			os << "," << attrValue << ";" << decision;
		}
		else if (type == DN_INNER_TYPE) {
			os << "," << attrValue;
		}
		os << ")";
	}

	void DecisionTree::addChild(DecisionTree* subTree)
	{
		if (firstChild == nullptr) {
			firstChild = subTree;
			return;
		}
		// find the most right tree, then insert subTree as its sibling
		DecisionTree* mostRightTree = firstChild;
		while (mostRightTree->nextSibling != nullptr) {
			mostRightTree = mostRightTree->nextSibling;
		}
		mostRightTree->nextSibling = subTree;
	}

	void DecisionTree::print(std::ostream& os, std::size_t spaceCnt) const
	{
		PrintSpace(os, spaceCnt);
		os << node << endl;;
		if (firstChild)
			firstChild->print(os, spaceCnt + 4);
		if (nextSibling)
			nextSibling->print(os, spaceCnt);
	}

	//求取该属性取特定值时的样本个数（如“温度”取“正常”时有2个样本）
	std::size_t GetValueCntOfAttr(const Samples& samples, const Attribute& attr, double attrValue)
	{
		size_t total = 0;
		// accumulate samples for attribute
		Samples::const_iterator last = samples.end();
		for (auto first = samples.begin(); first != last; ++first) {
			if (first->getAttrValue(attr) == attrValue) {
				++total;
			}
		}
		return total;
	}

	//求取属性的值种类（如“温度”有3个取值类型）
	std::size_t GetValueTypeCntOfAttr(const Samples& samples, const Attribute& attr)
	{
		set<size_t> valueTypeSet;
		// accumulate samples for attribute
		Samples::const_iterator last = samples.end();
		for (auto first = samples.begin(); first != last; ++first) {
			valueTypeSet.insert(first->getAttrValue(attr));
		}
		return valueTypeSet.size();
	}

	//求取该属性性取特定值时，结论（decision）为真的个数（如“温度”取“正常”时有0个为真的结论）
	std::size_t GetTrueCntForValueOfAttr(const Samples& samples, const Attribute& attr, double attrValue)
	{
		size_t trueCnt = 0;
		Samples::const_iterator last = samples.end();
		for (auto first = samples.begin(); first != last; ++first) {
			if (first->getAttrValue(attr) == attrValue) {
				if (first->getDecision())
					++trueCnt;
			}
		}
		return trueCnt;
	}

	//计算信息熵  H(体温{=高}) = -{2\over3}log_2{2\over3} - {1\over3}log_2{1\over3}
	double CalcEntropyOf(const Samples& samples, const Attribute& attr, double attrValue)
	{
		double total = GetValueCntOfAttr(samples, attr, attrValue);
		double trueResult = GetTrueCntForValueOfAttr(samples, attr, attrValue);
		// all has the same value
		if (total == trueResult || trueResult == 0)
			return 0;
		double trueRate = trueResult / total;
		double entropy = -(trueRate * log2(trueRate)) - ((1 - trueRate) * log2(1 - trueRate));
		return entropy;
	}

	//计算条件熵  H(样本集S|体温) = {2\over7} * H(体温{=正常}) + {3\over7} * H(体温{=高}) + {2\over7}H(体温_{=很高})
	double CalcConditionalEntropyOf(const Samples& samples, const Attribute& attr)
	{
		// calculate conditional entropy
		size_t valueTypeCnt = GetValueTypeCntOfAttr(samples, attr);
		double total = samples.size();
		double conditionalEntropy = 0;	
		for (size_t value = 0; value != valueTypeCnt; ++value) {
			conditionalEntropy += GetValueCntOfAttr(samples, attr, value)/ total * CalcEntropyOf(samples, attr, value);
		}
		return conditionalEntropy;
	}

	//找到最好的属性
	Attribute FindBestAttribute(const Samples& samples, const Attributes& attributes)
	{
		if (attributes.empty())
			throw runtime_error("empty attributes");
		// 找到最小条件熵的属性
		Attributes::const_iterator iter =
			min_element(attributes.begin(), attributes.end(), 
				[samples](const Attribute& lhs, const Attribute& rhs)
		{
			return CalcConditionalEntropyOf(samples, lhs) < CalcConditionalEntropyOf(samples, rhs);
		});
		return *iter;
	}

	//生成决策树
	DecisionTree* GenerateDecisionTree(const Samples& samples, const Attributes& attributes, const DecisionNode& root)
	{
		if (attributes.empty()) {
			DecisionTree* tree = new DecisionTree(root);
			return tree;
		}
		DecisionTree* tree = new DecisionTree(root);
		const Attribute bestAttr = FindBestAttribute(samples, attributes);
		const size_t valueTypeCnt = GetValueTypeCntOfAttr(samples, bestAttr); //最好属性的取值
		for (size_t value = 0; value != valueTypeCnt; ++value) {
			double entropy = CalcEntropyOf(samples, bestAttr, value);
			if (entropy == 0) { // already know the decision
				size_t trueCnt = GetTrueCntForValueOfAttr(samples, bestAttr, value);
				size_t decision = trueCnt > 0 ? 1 : 0;
				DecisionNode subTreeNode(DN_LEAF_TYPE, bestAttr, value, decision);
				DecisionTree* subTree = new DecisionTree(subTreeNode);
				tree->addChild(subTree);
			}
			else {
				DecisionNode subTreeNode(DN_INNER_TYPE, bestAttr, value);
				// generate new attribute list
				Attributes newAttrs(attributes);
				newAttrs.remove(bestAttr);
				// generate new Samples
				Samples newSamples = GenerateSamplesByAttrValue(	samples, bestAttr, value);
				DecisionTree* subTree = GenerateDecisionTree(newSamples, newAttrs, subTreeNode);
				tree->addChild(subTree);
			}
		}
		return tree;
	}

	std::shared_ptr<DecisionTree> GenerateDecisionTree(	const Samples& samples, const Attributes& attributes)
	{
		DecisionNode root(DN_ROOT_TYPE, "root");
		DecisionTree* tree = GenerateDecisionTree(samples, attributes, root);
		return shared_ptr<DecisionTree>(tree);
	}

}