#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>

using namespace std;

enum class SyntaxType
{
	ENope,
	EChangeValue,
	EMovePointer,
	EPrintValue,
	EReadValue,
	ELoopStart,
	ELoopEnd,
	//advanced
	EAssignValue
};

class SyntaxElem
{
public:
	virtual ~SyntaxElem() {}

	virtual void Print(string& result, int& deep) = 0;
	virtual SyntaxType Type() = 0;

protected:
	inline void PrintBegin(string& result, int& deep)
	{
		result.reserve(result.size() + deep + 1);
		int t = deep;
		result += '\n';

		while (t--)
		{
			result += '\t';
		}
	}
};

//advanced
class AssignValue : public SyntaxElem
{
public:
	AssignValue(int v)
		: value(v)
	{}

	void Print(string& result, int& deep) override
	{
		PrintBegin(result, deep);

		std::ostringstream os;
		os << "*p = " << value << ';';
		result += os.str();
	}

	SyntaxType Type()
	{
		return SyntaxType::EAssignValue;
	}

public:
	int value;
};

//basic
class ChangeValue : public SyntaxElem
{
public:
	ChangeValue(int v)
		: value(v)
	{}

	void Print(string& result, int& deep) override
	{
		if (value == 0)
			return;

		PrintBegin(result, deep);

		std::ostringstream os;

		if (value > 0)
		{
			os << "*p += " << value << ';';
		}
		else if (value < 0)
		{
			os << "*p -= " << -value << ';';
		}

		result += os.str();
	}

	SyntaxType Type()
	{
		return SyntaxType::EChangeValue;
	}

public:
	int value;
};

class MovePointer : public SyntaxElem
{
public:
	MovePointer(int v)
		: value(v)
	{}

	void Print(string& result, int& deep) override
	{
		if (value == 0)
			return;

		PrintBegin(result, deep);
		
		std::ostringstream os;
		
		if (value > 0)
		{
			os << "p += " << value << ';';
		}
		else if (value < 0)
		{
			os << "p -= " << -value << ';';
		}

		result += os.str();
	}

	SyntaxType Type()
	{
		return SyntaxType::EMovePointer;
	}

public:
	int value;
};

class PrintValue : public SyntaxElem
{
public:
	void Print(string& result, int& deep) override
	{
		PrintBegin(result, deep);
		result += "putchar(*p);";
	}

	SyntaxType Type()
	{
		return SyntaxType::EPrintValue;
	}
};

class ReadValue : public SyntaxElem
{
public:
	void Print(string& result, int& deep) override
	{
		PrintBegin(result, deep);
		result += "*p = getchar();";
	}

	SyntaxType Type()
	{
		return SyntaxType::EReadValue;
	}
};

class LoopStart : public SyntaxElem
{
public:
	void Print(string& result, int& deep) override
	{
		PrintBegin(result, deep);
		result += "while(*p) {";
		++deep;
	}

	SyntaxType Type()
	{
		return SyntaxType::ELoopStart;
	}
};

class LoopEnd : public SyntaxElem
{
public:
	void Print(string& result, int& deep) override
	{
		--deep;
		PrintBegin(result, deep);
		result += '}';
	}

	SyntaxType Type()
	{
		return SyntaxType::ELoopEnd;
	}
};

enum class CharType
{
	Nop,
	MovePointerForward,
	MovePointerBackward,
	IncrementValue,
	DecrementValue,
	PrintValue,
	ReadValue,
	LoopStart,
	LoopEnd
};

class Decompiler
{
private:
	char* in;
	list<SyntaxElem*> output;

public:
	Decompiler(char* input)
		: in(input)
	{
	}

	list<SyntaxElem*>& GetSyntaxTree()
	{
		return output;
	}

	string Print()
	{
		string o;
		int deep = 1;
		for (auto v : output)
		{
			v->Print(o, deep);
		}
		return o;
	}

	void Decompile()
	{
		long len = strlen(in);
		const char* p = in;

		while (*p)
		{
			int value = 0;

			SkipNils(p);

			while (*p && (ReadElemType(*p) == CharType::Nop 
				|| ReadElemType(*p) == CharType::MovePointerForward
				|| ReadElemType(*p) == CharType::MovePointerBackward))
			{
				CharType type = ReadElemType(*p);
				if (type == CharType::MovePointerForward)
					++value;
				if (type == CharType::MovePointerBackward)
					--value;
				p++;
			}
			if (value != 0)
			{
				output.push_back(new MovePointer(value));
				value = 0;
			}

			SkipNils(p);
			while (*p && (ReadElemType(*p) == CharType::Nop
				|| ReadElemType(*p) == CharType::IncrementValue
				|| ReadElemType(*p) == CharType::DecrementValue))
			{
				CharType type = ReadElemType(*p);
				if (type == CharType::IncrementValue)
					++value;
				if (type == CharType::DecrementValue)
					--value;
				p++;
			}
			if (value != 0)
			{
				output.push_back(new ChangeValue(value));
				value = 0;
			}
			SkipNils(p);

			if (*p && ReadElemType(*p) == CharType::PrintValue)
			{
				output.push_back(new PrintValue());
				p++;
			}

			if (*p && ReadElemType(*p) == CharType::ReadValue)
			{
				output.push_back(new ReadValue());
				p++;
			}

			if (*p && ReadElemType(*p) == CharType::LoopStart)
			{
				output.push_back(new LoopStart());
				p++;
			}

			if (*p && ReadElemType(*p) == CharType::LoopEnd)
			{
				output.push_back(new LoopEnd());
				p++;
			}
		}
	}

	void SkipNils(const char*& p)
	{
		while (*p && ReadElemType(*p) == CharType::Nop)
		{
			++p;
		}
	}

	CharType ReadElemType(char element)
	{
		switch (element)
		{
		case '>':
			return CharType::MovePointerForward;
		case '<':
			return CharType::MovePointerBackward;
		case '+':
			return CharType::IncrementValue;
		case '-':
			return CharType::DecrementValue;
		case '.':
			return CharType::PrintValue;
		case ',':
			return CharType::ReadValue;
		case '[':
			return CharType::LoopStart;
		case ']':
			return CharType::LoopEnd;
		default:
			return CharType::Nop;
		}
	}
};

class Optimizer
{
private:
	list<SyntaxElem*>& out;
public:
	Optimizer(list<SyntaxElem*>& tree)
		:out(tree)
	{}

	void Optimize()
	{
		bool changed = true;
		while (changed)
		{
			changed = false;
			changed |= ConnectPointerMoves();
			changed |= ConnectValueMoves();
			changed |= ConnectAssigment();
			changed |= FindZeroes();
		}
		
	}

	bool FindZeroes()
	{
		bool changed = false;
		for (auto a = out.begin(); a != out.end(); ++a)
		{
			auto b = a;
			if ((*b)->Type() != SyntaxType::ELoopStart)
				continue;
			
			++b;
			if (b == out.end() || (*b)->Type() != SyntaxType::EChangeValue)
				continue;
			
			ChangeValue* v = (ChangeValue*)(*b);
			if (v->value != -1)
				continue;
			
			++b;
			if (b == out.end() || (*b)->Type() != SyntaxType::ELoopEnd)
				continue;

			b = a;
			auto start = a;
			start++;
			for (int i = 0; i < 3; ++i)
			{
				delete *b;
				*b = nullptr;
				++b;
			}

			b = a;
			*b = new AssignValue(0);
			advance(b, 3);
			out.erase(start, b);
			changed = true;
		}
		return changed;
	}

	bool ConnectValueMoves()
	{
		bool changed = false;
		for (auto a = out.begin(); a != out.end();)
		{
			auto b = a;
			++b;
			if (b != out.end() && (*a)->Type() == SyntaxType::EChangeValue && (*b)->Type() == SyntaxType::EChangeValue)
			{
				ChangeValue* elema = (ChangeValue*)(*a);
				ChangeValue* elemb = (ChangeValue*)(*b);
				elema->value += elemb->value;
				out.erase(b);
				delete elemb;
				if (elema->value == 0) //remove a
				{
					b = a;
					++a;
					out.erase(b);
					delete elema;
				}
				changed = true;
			}
			else
			{
				++a;
			}
		}
		return changed;
	}

	bool ConnectPointerMoves()
	{
		bool changed = false;
		for (auto a = out.begin(); a != out.end();)
		{
			auto b = a;
			++b;
			if (b != out.end() && (*a)->Type() == SyntaxType::EMovePointer && (*b)->Type() == SyntaxType::EMovePointer)
			{
				MovePointer* elema = (MovePointer*)(*a);
				MovePointer* elemb = (MovePointer*)(*b);
				elema->value += elemb->value;
				out.erase(b);
				delete elemb;
				if (elema->value == 0) //remove a
				{
					b = a;
					++a;
					out.erase(b);
					delete elema;
				}
				changed = true;
			}
			else
			{
				++a;
			}
		}
		return changed;
	}

	bool ConnectAssigment()
	{
		bool changed = false;
		for (auto a = out.begin(); a != out.end();)
		{
			auto b = a;
			++b;
			if (b != out.end() && (*a)->Type() == SyntaxType::EAssignValue && (*b)->Type() == SyntaxType::EChangeValue)
			{
				MovePointer* elema = (MovePointer*)(*a);
				MovePointer* elemb = (MovePointer*)(*b);
				elema->value += elemb->value;
				out.erase(b);
				delete elemb;
				changed = true;
			}
			else
			{
				++a;
			}
		}
		return changed;
	}
};

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "int.txt out.cpp";
	}

	char* inputFile = argv[1];
	char* outputFile = argv[2];

	FILE* f = fopen(inputFile, "rb");
	fseek(f, 0, SEEK_END);
	long inputSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char* input = new char[inputSize+1];
	input[inputSize] = 0;
	fread(input, sizeof(char), inputSize, f);
	fclose(f);

	Decompiler d(input);
	d.Decompile();
	Optimizer o(d.GetSyntaxTree());
	o.Optimize();

	std::string output;
	output += "#include <stdio.h>\n\n";
	output += "char *p;\n";
	output += "int main() {\n";
	output += "\tp = new char[10000];\n";
	output += d.Print();
	output += "\n}";

	FILE* f2 = fopen(outputFile, "wb");
	fwrite(output.c_str(), 1, output.size(), f2);
	fclose(f2);
}