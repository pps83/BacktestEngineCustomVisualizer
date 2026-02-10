// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// _BacktestEngineCustomVisualizerService.cpp : Implementation of CBacktestEngineCustomVisualizerService

#include "stdafx.h"
#include "_EntryPoint.h"
#include "../../../src/symbol.h"
#include <boost/decimal.hpp>

char* toChars(char* ptr, char* ptrEnd, boost::int128::int128_t i128);
char* toChars(char* ptr, char* ptrEnd, boost::int128::uint128_t u128);

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::EvaluateVisualizedExpression(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
    )
{
    HRESULT hr;

    // This method is called to visualize a FILETIME variable. Its basic job is to create
    // a DkmEvaluationResult object. A DkmEvaluationResult is the data that backs a row in the
    // watch window -- a name, value, and type, a flag indicating if the item can be expanded, and
    // lots of other additional properties.

    Evaluation::DkmPointerValueHome* pPointerValueHome = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
    if (pPointerValueHome == nullptr)
    {
        // This sample only handles visualizing in-memory FILETIME structures
        return E_NOTIMPL;
    }

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    // Read the FILETIME value from the target process
    bool is_Symbol0 = false;
    bool is_Symbol1 = false;
    bool is_Symbol2 = false;

    bool is_decimal32 = false;
    bool is_decimal64 = false;
    bool is_decimal128 = false;
    bool is_decimal_fast32 = false;
    bool is_decimal_fast64 = false;
    bool is_decimal_fast128 = false;

    bool is_int128 = false;
    bool is_uint128 = false;

    Symbol0 sym0;
    Symbol1 sym1;
    Symbol2 sym2;

    boost::decimal::decimal32_t decimal32;
    boost::decimal::decimal64_t decimal64;
    boost::decimal::decimal128_t decimal128;
    boost::decimal::decimal_fast32_t decimal_fast32;
    boost::decimal::decimal_fast64_t decimal_fast64;
    boost::decimal::decimal_fast128_t decimal_fast128;

    boost::int128::int128_t int128;
    boost::int128::uint128_t uint128;

    char symStr[128] = {};

    CString expressionType;

    auto type = pRootVisualizedExpression->Type();
    DkmProcess* pTargetProcess = pVisualizedExpression->RuntimeInstance()->Process();


#define DO_CHECK(name, flag, var, conv)                                      \
    if (expressionType.Find(name) != -1)                                     \
    {                                                                        \
        flag = true;                                                         \
        UINT32 bytesRead = 0;                                                \
        hr = pTargetProcess->ReadMemory(pPointerValueHome->Address(),        \
            DkmReadMemoryFlags::None, (void*)&var, sizeof(var), &bytesRead); \
        if (bytesRead != sizeof(var))                                        \
            hr = -1;                                                         \
        if (SUCCEEDED(hr) && bytesRead == sizeof(var))                       \
        {                                                                    \
            conv;                                                            \
        }                                                                    \
    }                                                                        \

#define CHECK_SYMBOL(name, flag, var) DO_CHECK(name, flag, var, var.toStringImpl(symStr))

#define CHECK_DECIMAL(name, n) DO_CHECK(name, is_decimal##n, decimal##n, *to_chars(symStr, symStr + sizeof(symStr), decimal##n).ptr='\0')

#define CHECK_I128(name, var) DO_CHECK(name, is_##var, var, toChars(symStr, symStr + sizeof(symStr), var))

    if (type && type->Value())
    {
        expressionType = type->Value();

        CHECK_SYMBOL(L"Symbol0", is_Symbol0, sym0);
        CHECK_SYMBOL(L"Symbol1", is_Symbol1, sym1);
        CHECK_SYMBOL(L"Symbol2", is_Symbol2, sym2);

        CHECK_DECIMAL(L"decimal32", 32);
        CHECK_DECIMAL(L"decimal64", 64);
        CHECK_DECIMAL(L"decimal128", 128);
        CHECK_DECIMAL(L"decimal_fast32", _fast32);
        CHECK_DECIMAL(L"decimal_fast64", _fast64);
        CHECK_DECIMAL(L"decimal_fast128", _fast128);

        CHECK_I128(L"::int128_t", int128);
        CHECK_I128(L"::uint128_t", uint128);
    }
    else
        is_Symbol2 = true;

    if (FAILED(hr))
    {
        // If the bytes of the value cannot be read from the target process, just fall back to the default visualization
        return E_NOTIMPL;
    }

    size_t len = strlen(symStr);
    if (len == 0)
        strcpy(symStr, "???");
    else if (is_Symbol0 || is_Symbol1 || is_Symbol2)
    {
        for (size_t i = 0; i < len; ++i)
        {
            if (symStr[i] < '.' || symStr[i] > 'Z')
            {
                strcpy(symStr, "???");
                break;
            }
        }
    }

    //CString strValue(L"expressionType is: \"" + expressionType + L"\", value : " + CA2T(symStr));
    CString strValue(symStr);

    CString strEditableValue;

    // If we are formatting a pointer, we want to also show the address of the pointer
    if (pRootVisualizedExpression->Type() != nullptr && wcschr(pRootVisualizedExpression->Type()->Value(), '*') != nullptr)
    {
        // Make the editable value just the pointer string
        UINT64 address = pPointerValueHome->Address();
        if ((pTargetProcess->SystemInformation()->Flags()& DefaultPort::DkmSystemInformationFlags::Is64Bit) != 0)
        {
            strEditableValue.Format(L"0x%08x%08x", static_cast<DWORD>(address >> 32), static_cast<DWORD>(address));
        }
        else
        {
            strEditableValue.Format(L"0x%08x", static_cast<DWORD>(address));
        }

        // Prefix the value with the address
        CString strValueWithAddress;
        strValueWithAddress.Format(L"%s {%s}", static_cast<LPCWSTR>(strEditableValue), static_cast<LPCWSTR>(strValue));
        strValue = strValueWithAddress;
    }

    CComPtr<DkmString> pValue;
    hr = DkmString::Create(DkmSourceString(strValue), &pValue);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmString> pEditableValue;
    hr = DkmString::Create(strEditableValue, &pEditableValue);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmDataAddress> pAddress;
    hr = DkmDataAddress::Create(pVisualizedExpression->RuntimeInstance(), pPointerValueHome->Address(), nullptr, &pAddress);
    if (FAILED(hr))
    {
        return hr;
    }

    DkmEvaluationResultFlags_t resultFlags = DkmEvaluationResultFlags::Expandable;
    if (strEditableValue.IsEmpty())
    {
        // We only allow editing pointers, so mark non-pointers as read-only
        resultFlags |= DkmEvaluationResultFlags::ReadOnly;
    }

    CComPtr<DkmSuccessEvaluationResult> pSuccessEvaluationResult;
    hr = DkmSuccessEvaluationResult::Create(
        pVisualizedExpression->InspectionContext(),
        pVisualizedExpression->StackFrame(),
        pRootVisualizedExpression->Name(),
        pRootVisualizedExpression->FullName(),
        resultFlags,
        pValue,
        pEditableValue,
        pRootVisualizedExpression->Type(),
        DkmEvaluationResultCategory::Class,
        DkmEvaluationResultAccessType::None,
        DkmEvaluationResultStorageType::None,
        DkmEvaluationResultTypeModifierFlags::None,
        pAddress,
        nullptr,
        (DkmReadOnlyCollection<DkmModuleInstance*>*)nullptr,
        // This sample doesn't need to store any state associated with this evaluation result, so we
        // pass `DkmDataItem::Null()` here. A more complicated extension which had associated
        // state such as an extension which took over expansion of evaluation results would likely
        // create an instance of the extension's data item class and pass the instance here.
        // More information: https://github.com/Microsoft/ConcordExtensibilitySamples/wiki/Data-Container-API
        DkmDataItem::Null(),
        &pSuccessEvaluationResult
        );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppResultObject = pSuccessEvaluationResult.Detach();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::UseDefaultEvaluationBehavior(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Out_ bool* pUseDefaultEvaluationBehavior,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
    )
{
    HRESULT hr;

    // This method is called by the expression evaluator when a visualized expression's children are
    // being expanded, or the value is being set. We just want to delegate this back to the C++ EE.
    // So we need to set `*pUseDefaultEvaluationBehavior` to true and return the evaluation result which would
    // be created if this custom visualizer didn't exist.
    //
    // NOTE: If this custom visualizer supported underlying strings (no DkmEvaluationResultFlags::RawString),
    // this method would also be called when that is requested.

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr)
    {
        // This sample doesn't provide child evaluation results, so only root expressions are expected
        return E_NOTIMPL;
    }

    DkmInspectionContext* pParentInspectionContext = pVisualizedExpression->InspectionContext();

    CAutoDkmClosePtr<DkmLanguageExpression> pLanguageExpression;
    hr = DkmLanguageExpression::Create(
        pParentInspectionContext->Language(),
        DkmEvaluationFlags::TreatAsExpression,
        pRootVisualizedExpression->FullName(),
        DkmDataItem::Null(),
        &pLanguageExpression
        );
    if (FAILED(hr))
    {
        return hr;
    }

    // Create a new inspection context with 'DkmEvaluationFlags::ShowValueRaw' set. This is important because
    // the result of the expression is a FILETIME, and we don't want our visualizer to be invoked again. This
    // step would be unnecessary if we were evaluating a different expression that resulted in a type which
    // we didn't visualize.
    CComPtr<DkmInspectionContext> pInspectionContext;
    if (DkmComponentManager::IsApiVersionSupported(DkmApiVersion::VS16RTMPreview))
    {
        // If we are running in VS 16 or newer, use this overload...
        hr = DkmInspectionContext::Create(
            pParentInspectionContext->InspectionSession(),
            pParentInspectionContext->RuntimeInstance(),
            pParentInspectionContext->Thread(),
            pParentInspectionContext->Timeout(),
            DkmEvaluationFlags::TreatAsExpression |
            DkmEvaluationFlags::ShowValueRaw,
            pParentInspectionContext->FuncEvalFlags(),
            pParentInspectionContext->Radix(),
            pParentInspectionContext->Language(),
            pParentInspectionContext->ReturnValue(),
            (Evaluation::DkmCompiledVisualizationData*)nullptr,
            Evaluation::DkmCompiledVisualizationDataPriority::None,
            pParentInspectionContext->ReturnValues(),
            pParentInspectionContext->SymbolsConnection(),
            &pInspectionContext
            );
    }
    else
    {
        // Otherwise fall back to the Visual Studio 14 version
        hr = DkmInspectionContext::Create(
            pParentInspectionContext->InspectionSession(),
            pParentInspectionContext->RuntimeInstance(),
            pParentInspectionContext->Thread(),
            pParentInspectionContext->Timeout(),
            DkmEvaluationFlags::TreatAsExpression |
            DkmEvaluationFlags::ShowValueRaw,
            pParentInspectionContext->FuncEvalFlags(),
            pParentInspectionContext->Radix(),
            pParentInspectionContext->Language(),
            pParentInspectionContext->ReturnValue(),
            (Evaluation::DkmCompiledVisualizationData*)nullptr,
            Evaluation::DkmCompiledVisualizationDataPriority::None,
            pParentInspectionContext->ReturnValues(),
            &pInspectionContext
            );
    }
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<DkmEvaluationResult> pEEEvaluationResult;
    hr = pVisualizedExpression->EvaluateExpressionCallback(
        pInspectionContext,
        pLanguageExpression,
        pVisualizedExpression->StackFrame(),
        &pEEEvaluationResult
        );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppDefaultEvaluationResult = pEEEvaluationResult.Detach();
    *pUseDefaultEvaluationBehavior = true;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::GetChildren(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ UINT32 InitialRequestSize,
    _In_ Evaluation::DkmInspectionContext* pInspectionContext,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren,
    _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** ppEnumContext
    )
{
    // This sample delegates expansion to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::GetItems(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ Evaluation::DkmEvaluationResultEnumContext* pEnumContext,
    _In_ UINT32 StartIndex,
    _In_ UINT32 Count,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
    )
{
    // This sample delegates expansion to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::SetValueAsString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ DkmString* pValue,
    _In_ UINT32 Timeout,
    _Deref_out_opt_ DkmString** ppErrorText
    )
{
    // This sample delegates setting values to the C++ EE, so this method doesn't need to be implemented
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBacktestEngineCustomVisualizerService::GetUnderlyingString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ DkmString** ppStringValue
    )
{
    // FILETIME doesn't have an underlying string (no DkmEvaluationResultFlags::RawString), so this method
    // doesn't need to be implemented
    return E_NOTIMPL;
}
