#include "virtualmachine.h"
#include "astnode.h"
#include "helper.h"
#include "parsesqf.h"
#include "parseconfig.h"
#include "instruction.h"
#include "vmstack.h"
#include "commandmap.h"
#include "cmd.h"
#include "configdata.h"
#include "value.h"
#include "sidedata.h"
#include "groupdata.h"
#include "scriptdata.h"
#include "callstack_sqftry.h"
#include "sqfnamespace.h"
#include "innerobj.h"
#include "scalardata.h"
//#include "parsepp_handler.h"

#include <iostream>
#include <cwctype>
#include <sstream>

// #define DEBUG_VM_ASSEMBLY

#if !defined(_DEBUG) && defined(DEBUG_VM_ASSEMBLY)
#undef DEBUG_VM_ASSEMBLY
#endif // !_RELEASE



sqf::virtualmachine::virtualmachine(unsigned long long maxinst)
{
	mout = &std::cout;
	mwrn = &std::cerr;
	merr = &std::cerr;
	mhaltflag = false;
	minstcount = 0;
	mmaxinst = maxinst;
	mmainstack = std::make_shared<vmstack>();
	mactivestack = mmainstack;
	merrflag = false;
	mwrnflag = false;
	mmissionnamespace = std::make_shared<sqf::sqfnamespace>("missionNamespace");
	muinamespace = std::make_shared< sqf::sqfnamespace>("uiNamespace");
	mparsingnamespace = std::make_shared<sqf::sqfnamespace>("parsingNamespace");
	mprofilenamespace = std::make_shared<sqf::sqfnamespace>("profileNamespace");
	mperformclassnamechecks = true;
	mexitflag = false;
	mallowsleep = true;
	mplayer_obj = innerobj::create(this, "CAManBase", false);
	mcreatedtimestamp = system_time();
}
void sqf::virtualmachine::execute()
{
	mexitflag = false;
	while (!mexitflag && (!mspawns.empty() || !mmainstack->isempty() ))
	{
		mactivestack = mmainstack;
		performexecute();
		while (!mmainstack->isempty()) { mmainstack->dropcallstack(); }
		for (auto& it : mspawns)
		{
			mactivestack = it->stack();
			if (mallowsleep && mactivestack->isasleep())
			{
				if (mactivestack->get_wakeupstamp() <= virtualmachine::system_time())
				{
					mactivestack->wakeup();
				}
				else
				{
					continue;
				}
			}
			performexecute(150);
		}
		mspawns.remove_if([](std::shared_ptr<scriptdata> it) { return it->hasfinished(); });
	}
	mactivestack = mmainstack;
}
void sqf::virtualmachine::performexecute(size_t exitAfter)
{
	std::shared_ptr<sqf::instruction> inst;
	while (!mexitflag && exitAfter != 0 && !mactivestack->isasleep() && (inst = mactivestack->popinst(this)).get())
	{
		minstcount++;
		if (exitAfter > 0)
		{
			exitAfter--;
		}
		if (mmaxinst != 0 && mmaxinst == minstcount)
		{
			err() << "MAX INST COUNT REACHED (" << mmaxinst << ")" << std::endl;
			(*merr) << inst->dbginf("RNT") << merr_buff.str();
            err_clear();
			break;
		}
		inst->execute(this);
#ifdef DEBUG_VM_ASSEMBLY
		bool success;
		std::vector<std::shared_ptr<sqf::value>> vals;
		do {
			auto val = stack()->popval(success);
			if (success)
			{
				vals.push_back(val);
				if (val != nullptr)
				{
					std::cout << "[WORK]\t<" << sqf::type_str(val->dtype()) << ">\t" << val->as_string() << std::endl;
				}
				else
				{
					std::cout << "[WORK]\t<" << "EMPTY" << ">\t" << std::endl;
				}
			}
		} while (success);
		while (!vals.empty())
		{
			auto it = vals.back();
			vals.pop_back();
			stack()->pushval(it);
		}
#endif
		if (merrflag)
		{
			(*merr) << inst->dbginf("RNT") << merr_buff.str();
            err_clear();

			// Try to find a callstack_sqftry
			auto res = std::find_if(mactivestack->stacks_begin(), mactivestack->stacks_end(), [](std::shared_ptr<sqf::callstack> cs) -> bool {
				return cs->recover();
			});
			if (res == mactivestack->stacks_end())
			{
                err_clear();
				//Only for non-scheduled (and thus the mainstack)
				if (!mactivestack->isscheduled())
				{
					break;
				}
			}
			else
			{
				while (mactivestack->stacks_top() != *res)
				{
					mactivestack->dropcallstack();
				}
				auto sqftry = std::dynamic_pointer_cast<sqf::callstack_sqftry>(*res);
				sqftry->except(merr_buff.str());
                err_clear();
			}
		}
		
		if (mwrnflag)
		{
			(*mwrn) << inst->dbginf("WRN") << mwrn_buff.str();
			mwrn_buff.str(std::string());
			mwrnflag = false;
		}
        out_buffprint();
	}
}
std::string sqf::virtualmachine::dbgsegment(const char* full, size_t off, size_t length)
{
	std::stringstream sstream;
	size_t i = off < 15 ? 0 : off - 15;
	size_t len = 30 + length;
	if (i < 0)
	{
		len += i;
		i = 0;
	}
	for (size_t j = i; j < i + len; j++)
	{
		char wc = full[j];
		if (wc == '\0' || wc == '\n')
		{
			if (j < off)
			{
				i = j + 1;
			}
			else
			{
				len = j - i;
				break;
			}
		}
	}
	sstream << std::string(full + i, full + i + len) << std::endl
		<< std::string(off - i, ' ') << std::string(length == 0 ? 1 : length, '^') << std::endl;
	return sstream.str();
}
bool contains_nular(std::string ident)
{
	return sqf::commandmap::get().contains_n(ident);
}
bool contains_unary(std::string ident)
{
	return sqf::commandmap::get().contains_u(ident);
}
bool contains_binary(std::string ident, short p)
{
	auto flag = sqf::commandmap::get().contains_b(ident);
	if (flag && p > 0)
	{
		auto cmds = sqf::commandmap::get().getrange_b(ident);
		return cmds->front()->precedence() == p;
	}
	return flag;
}
short precedence(std::string s)
{
	auto srange = sqf::commandmap::get().getrange_b(s);
	if (!srange.get() || srange->empty())
	{
		return 0;
	}
	return srange->begin()->get()->precedence();
}

void navigate_sqf(const char* full, sqf::virtualmachine* vm, std::shared_ptr<sqf::callstack> stack, astnode node)
{

}
void navigate_pretty_print_sqf(const char* full, sqf::virtualmachine* vm, astnode& node, size_t depth)
{
	switch (node.kind)
	{
		case sqf::parse::sqf::sqfasttypes::BEXP1:
		case sqf::parse::sqf::sqfasttypes::BEXP2:
		case sqf::parse::sqf::sqfasttypes::BEXP3:
		case sqf::parse::sqf::sqfasttypes::BEXP4:
		case sqf::parse::sqf::sqfasttypes::BEXP5:
		case sqf::parse::sqf::sqfasttypes::BEXP6:
		case sqf::parse::sqf::sqfasttypes::BEXP7:
		case sqf::parse::sqf::sqfasttypes::BEXP8:
		case sqf::parse::sqf::sqfasttypes::BEXP9:
		case sqf::parse::sqf::sqfasttypes::BEXP10:
		case sqf::parse::sqf::sqfasttypes::BINARYEXPRESSION:
		{
			navigate_pretty_print_sqf(full, vm, node.children[0], depth);
			vm->out() << ' ' << node.children[1].content << ' ';
			navigate_pretty_print_sqf(full, vm, node.children[2], depth);
		}
		break;
		case sqf::parse::sqf::sqfasttypes::UNARYEXPRESSION:
		{
			vm->out() << node.children[0].content << ' ';
			navigate_pretty_print_sqf(full, vm, node.children[1], depth);
		}
		break;
		case sqf::parse::sqf::sqfasttypes::NUMBER:
		case sqf::parse::sqf::sqfasttypes::HEXNUMBER:
		case sqf::parse::sqf::sqfasttypes::NULAROP:
		case sqf::parse::sqf::sqfasttypes::STRING:
		case sqf::parse::sqf::sqfasttypes::VARIABLE:
		{
			vm->out() << node.content;
		}
		break;
		case sqf::parse::sqf::sqfasttypes::BRACKETS:
		{
			vm->out() << "(";
			navigate_pretty_print_sqf(full, vm, node.children[0], depth);
			vm->out() << ")";
		}
		break;
		case sqf::parse::sqf::sqfasttypes::CODE:
		{
			vm->out() << "{" << std::endl;
			depth++;
			for (auto& subnode : node.children)
			{
				vm->out() << std::string(depth * 4, ' ');
				navigate_pretty_print_sqf(full, vm, subnode, depth);
				vm->out() << ";" << std::endl;
			}
			depth--;
			vm->out() << std::string(depth * 4, ' ') << "}";
		}
		break;
		case sqf::parse::sqf::sqfasttypes::ARRAY:
		{
			vm->out() << "[";
			bool flag = false;
			for (auto& subnode : node.children)
			{
				if (flag)
				{
					vm->out() << ", ";
				}
				else
				{
					flag = true;
				}
				navigate_pretty_print_sqf(full, vm, subnode, depth);
			}
			vm->out() << "]";
		}
		break;
		case sqf::parse::sqf::sqfasttypes::ASSIGNMENT:
		{
			vm->out() << node.children[0].content << " = ";
			navigate_pretty_print_sqf(full, vm, node.children[1], depth);
		}
		break;
		case sqf::parse::sqf::sqfasttypes::ASSIGNMENTLOCAL:
		{
			vm->out() << "private " << node.children[0].content << " = ";
			navigate_pretty_print_sqf(full, vm, node.children[1], depth);
		}
		break;
		default:
		{
			for (auto& subnode : node.children)
			{
				vm->out() << std::string(depth, '\t');
				navigate_pretty_print_sqf(full, vm, subnode, depth);
				vm->out() << ";" << std::endl;
			}
		}
	}
}

astnode sqf::virtualmachine::parse_sqf_cst(std::string_view code, bool& errorflag, std::string filename)
{
	auto h = sqf::parse::helper(merr, dbgsegment, contains_nular, contains_unary, contains_binary, precedence);
	return sqf::parse::sqf::parse_sqf(code.data(), h, errorflag, filename);
}

void sqf::virtualmachine::parse_sqf_tree(std::string code, std::stringstream* sstream)
{
	auto h = sqf::parse::helper(merr, dbgsegment, contains_nular, contains_unary, contains_binary, precedence);
	bool errflag = false;
	auto node = sqf::parse::sqf::parse_sqf(code.c_str(), h, errflag, "");
	print_navigate_ast(sstream, node, sqf::parse::sqf::astkindname);
}

bool sqf::virtualmachine::parse_sqf(std::shared_ptr<sqf::vmstack> vmstck, std::string code, std::shared_ptr<sqf::callstack> cs, std::string filename)
{
	if (!cs.get())
	{
		cs = std::make_shared<sqf::callstack>(this->missionnamespace());
		vmstck->pushcallstack(cs);
	}
	auto h = sqf::parse::helper(&merr_buff, dbgsegment, contains_nular, contains_unary, contains_binary, precedence);
	bool errflag = false;
	auto node = sqf::parse::sqf::parse_sqf(code.c_str(), h, errflag, filename);
	this->merrflag = h.err_hasdata();
	if (!errflag)
	{
		navigate_sqf(code.c_str(), this, cs, node);
		errflag = this->err_hasdata();
	}
	return errflag;
}

void sqf::virtualmachine::pretty_print_sqf(std::string code)
{
	auto h = sqf::parse::helper(merr, dbgsegment, contains_nular, contains_unary, contains_binary, precedence);
	bool errflag = false;
	auto node = sqf::parse::sqf::parse_sqf(code.c_str(), h, errflag, "");
	if (!errflag)
	{
		navigate_pretty_print_sqf(code.c_str(), this, node, 0);
	}
}

void navigate_config(const char* full, sqf::virtualmachine* vm, std::shared_ptr<sqf::configdata> parent, astnode& node)
{
	auto kind = static_cast<sqf::parse::config::configasttypes::configasttypes>(node.kind);
	switch (kind)
	{
	case sqf::parse::config::configasttypes::CONFIGNODE:
	{
		std::shared_ptr<sqf::configdata> curnode;
		if (node.children.size() > 0 && node.children.front().kind == sqf::parse::config::configasttypes::CONFIGNODE_PARENTIDENT)
		{
			curnode = std::make_shared<sqf::configdata>(parent, node.content, node.children.front().content);
			for (size_t i = 1; i < node.children.size(); i++)
			{
				auto subnode = node.children[i];
				navigate_config(full, vm, curnode, subnode);
			}
		}
		else
		{
			curnode = std::make_shared<sqf::configdata>(parent, node.content);
			for (auto subnode : node.children)
			{
				navigate_config(full, vm, curnode, subnode);
			}
		}
		parent->push_back(std::make_shared<sqf::value>(curnode, sqf::type::CONFIG));
	} break;
	case sqf::parse::config::configasttypes::VALUENODE:
	{
		std::shared_ptr<sqf::configdata> curnode = std::make_shared<sqf::configdata>(parent, node.content);
		for (auto& subnode : node.children)
		{
			navigate_config(full, vm, curnode, subnode);
		}
		parent->push_back(std::make_shared<sqf::value>(curnode, sqf::type::CONFIG));
	} break;
	case sqf::parse::config::configasttypes::STRING:
		parent->set_cfgvalue(std::make_shared<sqf::value>(node.content));
		break;
	case sqf::parse::config::configasttypes::NUMBER:
		parent->set_cfgvalue(std::make_shared<sqf::value>(std::stod(node.content)));
		break;
	case sqf::parse::config::configasttypes::HEXNUMBER:
		parent->set_cfgvalue(std::make_shared<sqf::value>(std::stol(node.content, nullptr, 16)));
		break;
	case sqf::parse::config::configasttypes::LOCALIZATION:
		parent->set_cfgvalue(std::make_shared<sqf::value>(node.content));
		break;
	case sqf::parse::config::configasttypes::ARRAY:
	{
		std::vector<std::shared_ptr<sqf::value>> values;
		for (auto& subnode : node.children)
		{
			navigate_config(full, vm, parent, subnode);
			values.push_back(parent->cfgvalue());
		}
		parent->set_cfgvalue(std::make_shared<sqf::value>(values));
	} break;
	case sqf::parse::config::configasttypes::VALUE:
		break;
	default:
	{
		for (auto& subnode : node.children)
		{
			navigate_config(full, vm, parent, subnode);
		}
	}
	}
}
void sqf::virtualmachine::parse_config(std::string code, std::shared_ptr<configdata> parent)
{
	auto h = sqf::parse::helper(merr, dbgsegment, contains_nular, contains_unary, contains_binary, precedence);
	bool errflag = false;
	auto node = sqf::parse::config::parse_config(code, h, errflag);
//#if defined(_DEBUG)
//	static bool isinitial = true;
//	if (isinitial)
//	{
//		isinitial = false;
//		out() << "-------------------------------" << std::endl;
//		print_navigate_ast(mout, node, sqf::parse::config::astkindname);
//		out() << "-------------------------------" << std::endl;
//	}
//#endif

	if (!errflag)
	{
		navigate_config(code.c_str(), this, parent, node);
	}
}

size_t sqf::virtualmachine::push_obj(std::shared_ptr<sqf::innerobj> obj)
{
	if (mfreeobjids.size() != 0)
	{
		auto id = mfreeobjids.back();
		mfreeobjids.pop_back();
		mobjlist[id] = obj;
		return id;
	}
	else
	{
		auto id = mobjlist.size();
		mobjlist.push_back(obj);
		return id;
	}
}
void sqf::virtualmachine::drop_obj(const sqf::innerobj * obj)
{
	auto id = obj->netid();
	mobjlist[id] = std::shared_ptr<sqf::innerobj>();
	mfreeobjids.push_back(id);
}

std::shared_ptr<sqf::innerobj> sqf::virtualmachine::get_obj_netid(size_t netid)
{
	if (mobjlist.size() <= netid)
	{
		return std::shared_ptr<innerobj>();
	}
	return mobjlist[netid];
}

std::string sqf::virtualmachine::get_group_id(std::shared_ptr<sqf::sidedata> side)
{
	int sidenum = side->side();
	int id = mgroupidcounter[sidenum]++;
	std::stringstream sstream;
	sstream << side->tosqf() << " ALPHA " << id;
	return sstream.str();
}

void sqf::virtualmachine::push_group(std::shared_ptr<sqf::groupdata> d)
{
	mgroups[d->side()->side()].push_back(d);
}

void sqf::virtualmachine::drop_group(std::shared_ptr<sqf::groupdata> grp)
{
	auto& grpList = mgroups[grp->side()->side()];
	for (size_t i = 0; i < grpList.size(); i++)
	{
		if (grpList[i].get() == grp.get())
		{
			grpList[i] = grpList.back();
			grpList.pop_back();
			return;
		}
	}
}
std::chrono::system_clock::time_point sqf::virtualmachine::system_time()
{
	return std::chrono::system_clock::now();
}
//std::string sqf::virtualmachine::preprocess_file(std::string inputfile)
//{
//	//parse::preprocessor::ppparser parser;
//	//return parser.parse(this, inputfile);
//	return "";
//}