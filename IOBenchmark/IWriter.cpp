#include "IWriter.h"
#include "WriterAsync.h"
#include "WriterBasic.h"

using namespace std;

/*static*/std::shared_ptr<IWriter> IWriter::CreateInstance(WriterType type)
{
	switch (type)
	{
	case IWriter::WriterType::SimpleSync:
		return make_shared<WriterBasic>();
	case IWriter::WriterType::Async:
		return make_shared<WriterAsync>();
	default:
		return std::shared_ptr<IWriter>();
	}
}