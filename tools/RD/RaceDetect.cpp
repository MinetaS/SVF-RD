#include "RD/RDGeneric.h"
#include "RD/LockObject.h"
#include "RD/PointsToMap.h"

#include "MSSA/MemPartition.h"
#include "MSSA/MemRegion.h"
#include "Graphs/SVFG.h"
#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/PAGBuilder.h"
#include "WPA/Andersen.h"

#include <fstream>
#include <sstream>
#include <queue>
#include <vector>

/* Abusing keyword 'using namespace' is inappropriate and I won't use them here */
// using namespace llvm;
// using namespace std;
using namespace SVF;

using std::queue;
using std::string;
using std::vector;

using MUSet = MemSSA::MUSet;
using CHISet = MemSSA::CHISet;

/* LLVM options */
static llvm::cl::opt<string> InputFilename(llvm::cl::Positional, llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));
static llvm::cl::opt<string> LockListFilename(llvm::cl::Optional, "ldb", llvm::cl::desc("list of lock/unlock function names"), llvm::cl::init(""));
static llvm::cl::opt<bool> DebugRD(llvm::cl::Optional, "debug-rd", llvm::cl::desc("enable debug output"), llvm::cl::init(false));

/* core functions */
vector<string> ParseArguments(int argc, char **argv);

/* algorithm functions and macro definitions */
bool TraverseICFG(ICFG &icfg, Lock &lock);
bool TraverseICFG_DFS(ICFGNode *node, Lock &lock, Set<NodeID> &visited);

// The array of synchronization range IDs creatd and stored within
// every llvm::SmallVector which required to be specified the maximum size.
#define EXPECTED_MAX_SYNC_RANGES 8

/* LLVM util functions */
llvm::Function *GetCalleeFunction(llvm::Instruction &inst);
bool IsLLVMIntrinsicFunction(llvm::Instruction &inst);
bool IsStackAccess(llvm::Value *val);
std::string GetInstructionLocation(llvm::Instruction &inst);


// Global map: function names <--> IDs
LockFunctionMap lock_functions;


int main(int argc, char **argv) {
    /* ==================== Step 0 ==================== */
    // - Parse arguments
    // - Initialize some graphs
    dout << "==================== STEP 0 ====================\n";

    vector<string> module_names = ParseArguments(argc, argv);
    SVFModule *module = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(module_names);
    PAG *PAG = PAGBuilder().build(module);
    ICFG *ICFG = PAG->getICFG();
    Andersen *AndersenPTA = AndersenWaveDiff::createAndersenWaveDiff(PAG);
    SVFG *SVFG = SVFGBuilder().buildFullSVFG(AndersenPTA);

    /* ==================== Step 1 ==================== */
    // - It checks all of callsites and match lock-unlock functions;
    //   traverse ICFG and mark with an unique ID to identify synchronization status afterwards.
    // - Adds instructions to ptsMap
    out << "==================== STEP 1 ====================\n";

    MemSSA *MSSA = SVFG->getMSSA();
    PointsToMap ptsMap;

    for (SVFModule::iterator fit = module->begin() ; fit != module->end() ; ++fit) {
        const SVFFunction *function = *fit;
        out << "FUNCTION: " << function->getName() << "\n";

        for (llvm::Function::iterator bit = function->getLLVMFun()->begin() ; bit != function->getLLVMFun()->end() ; ++bit) {
            llvm::BasicBlock &bb = *bit;

            for (llvm::BasicBlock::iterator iit = bb.begin() ; iit != bb.end() ; ++iit) {
                llvm::Instruction &inst = *iit;

                if (SVFUtil::isCallSite(&inst)) {
                    // skip LLVM intrinsic call instructions
                    if (IsLLVMIntrinsicFunction(inst)) {
                        continue;
                    }

                    llvm::Function *callee = GetCalleeFunction(inst);
                    CallBlockNode *cb = ICFG->getCallBlockNode(&inst);

                    if (MSSA->hasMU(cb)) {
                        MUSet &set = MSSA->getMUSet(cb);

                        for (MUSet::iterator it = set.begin() ; it != set.end() ; ++it) {
                            PointsTo &pts = const_cast<PointsTo &>((*it)->getMR()->getPointsTo());

                            for (PointsTo::iterator ptsit = pts.begin() ; ptsit != pts.end() ; ++ptsit) {
                                ptsMap.addInstructionToLoadSet(*ptsit, &inst);
                            }
                        }
                    }

                    if (MSSA->hasCHI(cb)) {
                        CHISet &set = MSSA->getCHISet(cb);

                        for (CHISet::iterator it = set.begin() ; it != set.end() ; ++it) {
                            PointsTo &pts = const_cast<PointsTo &>((*it)->getMR()->getPointsTo());

                            for (PointsTo::iterator ptsit = pts.begin() ; ptsit != pts.end() ; ++ptsit) {
                                ptsMap.addInstructionToStoreSet(*ptsit, &inst);
                            }
                        }
                    }

                    if (!callee) {
                        continue;
                    }

                    // check function name
                    if (callee->hasName()) {
                        string name = callee->getName().str();

                        if (!DisableAnalysis && lock_functions.exists(name)) {
                            FuncID l_id = lock_functions.getID(name);
                            FuncType l_type = lock_functions.getType(name);

                            if (l_type == TYPE_LOCK) {
                                // no parameters = global synchronization
                                if (callee->arg_empty()) {
                                    Lock lock(l_id, l_id, &inst);
                                    if (TraverseICFG(*ICFG, lock)) {
                                        dout << "  Path found: Lock ID " << lock.getID()
                                            << " - locations: [[ lock: " << GetInstructionLocation(*lock.getLockInstruction())
                                            << ", unlock: " << GetInstructionLocation(*lock.getUnlockInstruction())
                                            << " ]]\n";
                                    }
                                }
                                // local synchronization otherwise
                                else {
                                    dout << "Not implemented yet: local function " << name << "\n";
                                }
                            }
                        }
                        else if (name.find("lock") != string::npos) {
                            out << "  Is this a lock function?\n";
                            out << "    name: " << name << ", location: [[ " << GetInstructionLocation(inst) << " ]]\n";
                        }
                        
                    }
                }
                else {
                    if (IsStackAccess(&inst)) {
                        continue;
                    }

                    PAG::PAGEdgeList &edges = MSSA->getMRGenerator()->getPAGEdgesFromInst(&inst);

                    for (PAG::PAGEdgeList::iterator it = edges.begin() ; it != edges.end() ; ++it) {
                        const PAGEdge *edge = *it;
                        const LoadPE *edge_load = llvm::dyn_cast<LoadPE>(edge);
                        const StorePE *edge_store = llvm::dyn_cast<StorePE>(edge);

                        if (edge_load) {
                            MUSet &set = MSSA->getMUSet(edge_load);

                            for (MUSet::iterator it = set.begin() ; it != set.end() ; ++it) {
                                PointsTo &pts = const_cast<PointsTo &>((*it)->getMR()->getPointsTo());

                                for (PointsTo::iterator ptsit = pts.begin() ; ptsit != pts.end() ; ++ptsit) {
                                    ptsMap.addInstructionToLoadSet(*ptsit, &inst);
                                }
                            }
                        }

                        if (edge_store) {
                            CHISet &set = MSSA->getCHISet(edge_store);

                            for (CHISet::iterator it = set.begin() ; it != set.end() ; ++it) {
                                PointsTo &pts = const_cast<PointsTo &>((*it)->getMR()->getPointsTo());

                                for (PointsTo::iterator ptsit = pts.begin() ; ptsit != pts.end() ; ++ptsit) {
                                    ptsMap.addInstructionToStoreSet(*ptsit, &inst);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* ==================== Step 2 ==================== */
    // - Print all possible pairs in ptsMap
    out << "==================== STEP 2 ====================\n";

    return 0;
}

vector<string> ParseArguments(int argc, char **argv) {
    int _argc = 0;
    char **_argv = new char *[argc];
    vector<string> modules;

    bool foundIR = false;

    for (int i=0 ; i<argc ; ++i) {
        string arg(argv[i]);

        if (SVFUtil::isIRFile(arg)) {
            if (find(modules.cbegin(), modules.cend(), arg) == modules.cend()) {
                modules.push_back(arg);
            }

            if (!foundIR) {
                _argv[_argc++] = argv[i];
                foundIR = true;
            }
        }
        else {
            _argv[_argc++] = argv[i];
        }
    }

    llvm::cl::ParseCommandLineOptions(_argc, _argv, "Race detecton based on MemorySSA and SVFG\n");
    delete[] _argv;

    // --debug-rd
    RtDebugMode = DebugRD.getValue();

    // --ldb=<filename>
    string ldb_filename = LockListFilename.getValue();

    if (ldb_filename.empty()) {
        out << "[-] --ldb option not set, analysis is disabled.\n";
        DisableAnalysis = true;
    }
    else {
        std::ifstream ldb_file(ldb_filename);

        if (!ldb_file.is_open()) {
            out << "[!] Cannot open the file: " << ldb_filename << "\n";
            exit(EXIT_FAILURE);
        }

        LockID max = 0;
        int lc = 1;
        string line;

        while (!ldb_file.eof()) {
            FuncID func_id;
            bool func_type;
            string func_name;

            std::getline(ldb_file, line);
            std::stringstream _ss(line);

            _ss >> func_id >> func_type >> func_name;

            if (!_ss.eof() || _ss.fail()) {
                out << "[-] Invalid value found. Line " << lc << ": " << line << " ignored.\n";
                continue;
            }

            if (func_id < 1) {
                out << "[-] Function ID must be above zero. Line " << lc << ": " << line << " ignored.\n";
                continue;
            }

            lock_functions.addObject(func_id, func_type, func_name);

            if (func_id > max) {
                max = func_id;
            }

            ++lc;
        }

        ldb_file.close();
        Lock::updateID(max+1);
    }

    return modules;
}

bool TraverseICFG(ICFG &icfg, Lock &lock) {
    InstID inst = lock.getLockInstruction();
    ICFGNode *node = icfg.getBlockICFGNode(inst);

    Set<NodeID> visited;

    return TraverseICFG_DFS(node, lock, visited);
}

bool TraverseICFG_DFS(ICFGNode *node, Lock &lock, Set<NodeID> &visited) {
    bool result = false;
    llvm::BasicBlock *bb = const_cast<llvm::BasicBlock *>(node->getBB());

    if (bb && node->getNodeKind() == ICFGNode::ICFGNodeK::FunCallBlock) {
        for (llvm::BasicBlock::iterator it = bb->begin() ; it != bb->end() ; ++it) {
            llvm::Instruction &__i = *it;

            if (SVFUtil::isCallSite(&__i)) {
                llvm::Function *callee = GetCalleeFunction(__i);

                if (!callee || !callee->hasName()) {
                    continue;
                }

                string name = callee->getName().str();

                if (lock_functions.exists(name)) {
                    FuncID l_id = lock_functions.getID(name);
                    FuncType l_type = lock_functions.getType(name);

                    if (l_type == TYPE_UNLOCK && l_id == lock.getFunctionID()) {
                        lock.setUnlockInstruction(const_cast<InstID>(&__i));
                        result = true;
                        break;
                    }
                }
            }
        }
    }

    if (!result && node->hasOutgoingEdge()) {
        for (ICFGNode::const_iterator it = node->OutEdgeBegin() ; it != node->OutEdgeEnd() ; ++it) {
            ICFGEdge *edge = *it;
            ICFGNode *next = edge->getDstNode();

            if (visited.find(next->getId()) == visited.end()) {
                visited.insert(next->getId());
                if (TraverseICFG_DFS(next, lock, visited) && !result) {
                    result = true;
                    visited.erase(next->getId());
                }
            }
        }
    }

    if (result) {
        llvm::BasicBlock &bb = *const_cast<llvm::BasicBlock *>(node->getBB());
        llvm::LLVMContext &ctx = bb.getContext();

        for (llvm::BasicBlock::iterator it = bb.begin() ; it != bb.end() ; ++it) {
            llvm::Instruction &inst = *it;

            llvm::SmallVector<llvm::Metadata *, EXPECTED_MAX_SYNC_RANGES> ops_vec;
            llvm::MDTuple *m;
            LockID id = Lock::getLastID();
            bool id_new = false;

            if (m = static_cast<llvm::MDTuple *>(inst.getMetadata("rd.sync_status"))) {
                for (llvm::MDTuple::op_iterator oit = m->op_begin() ; oit != m->op_end() ; ++oit) {
                    const llvm::MDOperand &op = *oit;
                    ops_vec.push_back(op.get());

                    if (!id_new) {
                        uint64_t __v = static_cast<llvm::ConstantInt *>(static_cast<llvm::ConstantAsMetadata *>(op.get())->getValue())->getValue().getLimitedValue();
                        if (id == __v) {
                            id_new = true;
                        }
                    }
                }
            }

            if (!id_new) {
                ops_vec.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(ctx, llvm::APInt(64, id))));
            }

            llvm::MDTuple::get(ctx, ops_vec);
        }
    }

    return result;
}

llvm::Function *GetCalleeFunction(llvm::Instruction &inst) {
    const SVFFunction *icallee = SVFUtil::getCallee(&inst);

    if (!icallee) {
        return nullptr;
    }

    return icallee->getLLVMFun();
}

bool IsLLVMIntrinsicFunction(llvm::Instruction &inst) {
    if (!SVFUtil::isCallSite(&inst)) {
        return false;
    }

    const llvm::CallBase *call_inst = llvm::dyn_cast<llvm::CallBase>(&inst);
    const llvm::Function *func = call_inst->getCalledFunction();

    if (!func) {
        return false;
    }

    return func->getIntrinsicID() != llvm::Intrinsic::not_intrinsic;
}

bool IsStackAccess(llvm::Instruction *inst) {
	if (llvm::AllocaInst *ai = llvm::dyn_cast<llvm::AllocaInst>(inst)) {
		return true;
	} else if (llvm::StoreInst *si = llvm::dyn_cast<llvm::StoreInst>(inst)) {
		return IsStackAccess(si->getPointerOperand());
	} else if (llvm::LoadInst *li = llvm::dyn_cast<llvm::LoadInst>(inst)) {
		return IsStackAccess(li->getPointerOperand());
	} else if (llvm::CastInst *ci = llvm::dyn_cast<llvm::CastInst>(inst)) {
		return IsStackAccess(ci->getOperand(0));
	}

	return false;
}

std::string GetInstructionLocation(llvm::Instruction &inst) {
    if (const llvm::DebugLoc &loc = inst.getDebugLoc()) {
        std::string result;
        llvm::raw_string_ostream ss(result);
        loc.print(ss);
        ss.flush();
        return result;
    }
    else {
        return "unknown";
    }
}