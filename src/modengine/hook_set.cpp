#include "hook_set.h"

namespace modengine {

void HookSet::install(std::shared_ptr<Hook<GenericFunctionPointer>> hook)
{
    hooks.push_back(hook);
}

bool HookSet::hook_all()
{
    auto txn_status = DetourTransactionBegin();

    if (txn_status != NO_ERROR) {
        throw std::runtime_error("Unable to create a Detours transaction");
    }

    for (const auto& hook : hooks) {
        if (hook->applied) {
            continue;
        }

        txn_status = DetourAttach((PVOID*)&hook->original, (PVOID)hook->replacement);

        if (txn_status != NO_ERROR) {
            (void)DetourTransactionAbort();
            break;
        }

        hook->applied = true;
    }

    txn_status = DetourTransactionCommit();
    return txn_status == NO_ERROR;
}

bool HookSet::unhook_all()
{
    DetourTransactionBegin();

    for(const auto& hook : hooks) {
        DetourDetach((PVOID*)&hook->original, (PVOID)hook->replacement);
    }

    DetourTransactionCommit();
    return true;
}

}