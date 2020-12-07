#include "RD/RDGeneric.h"
#include "RD/LockObject.h"

#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/PAGBuilder.h"
#include "WPA/Andersen.h"

/* Abusing keyword 'using namespace' is inappropriate and I won't use them here */
// using namespace llvm;
// using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional, llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));
static llvm::cl::opt<bool> DebugRD("debug-rd", llvm::cl::desc("<input bitcode>"), llvm::cl::init(false));

bool isLLVMIntrinsicFunction(llvm::Instruction &inst) {
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

int main(int argc, char **argv) {
    // parse args
    int arg_num = 0;
    char **arg_value = new char *[argc];
    std::vector<std::string> module_names;

    SVFUtil::processArguments(argc, argv, arg_num, arg_value, module_names);
    llvm::cl::ParseCommandLineOptions(arg_num, arg_value, "Race detecton based on MemorySSA and SVFG\n");

    delete[] arg_value;

    RtDebugMode = DebugRD.getValue();

    // initialize data and graphs
    SVFModule *svf = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(module_names);
	PAGBuilder pag_builder;
	PAG *pag = pag_builder.build(svf);
    Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    PTACallGraph *cg = ander->getPTACallGraph();
    ICFG *icfg = pag->getICFG();

    SVFGBuilder svfg_builder;
    SVFG *svfg = svfg_builder.buildFullSVFG(ander);

    MemSSA *mssa = svfg->getMSSA();

    /* Lock locations & IDs */
    std::vector<Lock> lock_locations;

    for (SVFModule::iterator fit = svf->begin() ; fit != svf->end() ; ++fit) {
        const SVFFunction *function = *fit;
        out << "==================== FUNCTION: " << function->getName() << " ====================\n";

        for (llvm::Function::iterator bit = function->getLLVMFun()->begin() ; bit != function->getLLVMFun()->end() ; ++bit) {
            llvm::BasicBlock &bb = *bit;

            for (llvm::BasicBlock::iterator iit = bb.begin() ; iit != bb.end() ; ++iit) {
                llvm::Instruction &inst = *iit;

                if (SVFUtil::isCallSite(&inst)) {
                    // skip LLVM intrinsic call instructions
                    if (isLLVMIntrinsicFunction(inst)) {
                        continue;
                    }

                    const SVFFunction *callee = SVFUtil::getCallee(&inst);

                    if (!callee) {
                        continue;
                    }

                    // check function name
                    if (callee->getLLVMFun()->hasName()) {
                        std::string func_name = callee->getName().str();
                        int pos = func_name.find("lock");

                        if (pos == string::npos) {
                            continue;
                        }

                        // name: unlock
                        if (pos >= 2 && func_name.substr(pos-2, 2) == "un") {
                            LockID id = Lock::addObject(func_name, true);
                            lock_locations.push_back(Lock(&inst, id, false));
                        }
                        // name: lock
                        else {
                            LockID id = Lock::addObject(func_name);
                            lock_locations.push_back(Lock(&inst, id, true));
                        }

                        if (const llvm::DebugLoc &loc = inst.getDebugLoc()) {
                            out << "\tLocation: [[";
                            loc.print(out);
                            out << "]]\n";
                        }
                    }
                }
                else {
                    //
                }
            }
        }
    }

    /* Iterate lock locations and follow ICFG *//*
    for (std::vector<llvm::Instruction>::const_iterator it = lock_locations.cbegin() ; it != lock_locations.cend() ; ++it) {
        
    }*/
    return 0;
}