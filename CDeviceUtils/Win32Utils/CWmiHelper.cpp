#include "CWmiHelper.h"
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

#define DATETIME_FORMAT         _T("%4hd%2hd%2hd%2hd%2hd%2hd")

// https://learn.microsoft.com/zh-cn/windows/win32/wmisdk/numbers
// 数据类型     自动化类型      说明
// sint8        VT_I2           8 位有符号整数。
// sint16       VT_I2           16 位有符号整数。
// sint32       VT_I4           32 位有符号整数。
// sint64       VT_BSTR         字符串形式的有符号 64 位整数。 根据美国国家标准协会 (ANSI) C 规则，此类型遵循十六进制或十六进制格式。
// real32       VT_R4           遵循电气和电子工程师协会 (IEEE) 标准的 4 字节浮点值。
// real64       VT_R8           遵循 IEEE 标准的 8 字节浮点值。
// uint8        VT_UI1          8 位无符号整数。
// uint16       VT_I4           16 位无符号整数。
// uint32       VT_I4           32 位无符号整数。
// uint64       VT_BSTR         字符串形式的无符号 64 位整数。 根据 ANSI C 规则，此类型遵循十六进制或十进制格式。
// DATETIME     VT_BSTR         

static std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str)
{
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strResult(cbMultiByte, 0);
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);
    strResult.resize(nConverted);
    return strResult;
}

static std::wstring _MultiStrToWStr(UINT CodePage, const std::string& str)
{
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
    std::wstring strResult(cchWideChar, 0);
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());
    strResult.resize(nConverted);
    return strResult;
}

static std::wstring TStrToWStr(const _tstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _MultiStrToWStr(CP_ACP, str);
#endif
}

static wmi_string WStrToTStr(const std::wstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _WStrToMultiStr(CP_ACP, str);
#endif
}

void CWmiClassHelper::ConsolePrint(LPCTSTR pFormat, ...) const
{
    size_t nCchCount = MAX_PATH;
    wmi_string strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
            ::WriteConsole(console, strResult.c_str(), nSize, NULL, NULL);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= INT32_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);
}

void CWmiClassHelper::_PrintProperty(long lFlags) const
{
    HRESULT hr = S_OK;
    SAFEARRAY* arrayNames;

    if (!m_pObj)
    {
        return;
    }

    hr = m_pObj->GetNames(NULL, lFlags, NULL, &arrayNames);
    if (FAILED(hr))
    {
        return;
    }

    //获取数组维数
    UINT uDim = ::SafeArrayGetDim(arrayNames);

    LONG lLBound = 0;
    LONG lUBound = 0;

    //取得nDim维度的上界
    ::SafeArrayGetLBound(arrayNames, uDim, &lLBound);

    //取得nDim维度的下界
    ::SafeArrayGetUBound(arrayNames, uDim, &lUBound);

    BSTR* pvData;
    ::SafeArrayAccessData(arrayNames, (void**)&pvData);
    for (int i = 0; i <= lUBound - lLBound; i++)
    {
        wmi_string strName = WStrToTStr(*pvData).c_str();
        _Print(strName);
        ConsolePrint(_T("\r\n"));
        pvData++;
    }
    ::SafeArrayUnaccessData(arrayNames);
}

void CWmiClassHelper::_Print(const CComVariant& val) const
{
    if (VARENUM::VT_ARRAY & val.vt)
    {
        ConsolePrint(_T("{"));
        _PrintList(val);
        ConsolePrint(_T("}"));
    }
    else
    {
        switch (val.vt)
        {
        case VARENUM::VT_EMPTY:
            ConsolePrint(_T("EMPTY"));
            break;
        case VARENUM::VT_NULL:
            ConsolePrint(_T("NULL"));
            break;
        case VARENUM::VT_BOOL:
            ConsolePrint(_T("%s"), val.boolVal ? _T("TRUE") : _T("FALSE"));
            break;
        case VARENUM::VT_I1:
        case VARENUM::VT_UI1:
        case VARENUM::VT_UI2:
        case VARENUM::VT_I2:
        case VARENUM::VT_I4:
        case VARENUM::VT_INT:
            ConsolePrint(_T("%d"), val.intVal);
            break;
        case VARENUM::VT_UI4:
        case VARENUM::VT_UINT:
            ConsolePrint(_T("%u"), val.uintVal);
            break;
        case VARENUM::VT_I8:
            ConsolePrint(_T("%lld"), val.llVal);
            break;
        case VARENUM::VT_UI8:
            ConsolePrint(_T("%llu"), val.ullVal);
            break;
        case VARENUM::VT_R4:
            ConsolePrint(_T("%f"), val.fltVal);
            break;
        case VARENUM::VT_R8:
            ConsolePrint(_T("%lf"), val.dblVal);
            break;
        case VARENUM::VT_BSTR:
            ConsolePrint(_T(R"("%s")"), WStrToTStr(val.bstrVal).c_str());
            break;
        }
    }
}

void CWmiClassHelper::_PrintList(const CComVariant& val) const
{
    if (VARENUM::VT_ARRAY & val.vt)
    {
        SAFEARRAY* pArray = val.parray;
        LONG lLBound = 0;
        LONG lUBound = 0;
        UINT uDim = ::SafeArrayGetDim(pArray);

        //暂且只处理一维数组
        if (1 != uDim)
        {
            return;
        }

        UINT uElemsize = ::SafeArrayGetElemsize(pArray);
        ::SafeArrayGetLBound(pArray, uDim, &lLBound);
        ::SafeArrayGetUBound(pArray, uDim, &lUBound);
        LONG lCount = lUBound - lLBound;

        void* pElem = nullptr;
        ::SafeArrayAccessData(pArray, (void**)&pElem);

        size_t pAddr = (uint64_t)pElem;
        VARTYPE vtElem = val.vt & ~VARENUM::VT_ARRAY;
        for (LONG index = 0; index < lCount; index++)
        {
            switch (vtElem)
            {
            case VARENUM::VT_I1:
                ConsolePrint(_T("%d"), *(int8_t*)pAddr);
                break;
            case VARENUM::VT_UI1:
                ConsolePrint(_T("%d"), *(uint8_t*)pAddr);
                break;
            case VARENUM::VT_I2:
                ConsolePrint(_T("%hd"), *(int16_t*)pAddr);
                break;
            case VARENUM::VT_UI2:
                ConsolePrint(_T("%hu"), *(uint16_t*)pAddr);
                break;
            case VARENUM::VT_I4:
            case VARENUM::VT_INT:
                ConsolePrint(_T("%d"), *(int32_t*)pAddr);
                break;
            case VARENUM::VT_UI4:
            case VARENUM::VT_UINT:
                ConsolePrint(_T("%u"), *(uint32_t*)pAddr);
                break;
            case VARENUM::VT_I8:
                ConsolePrint(_T("%lld"), *(int32_t*)pAddr);
                break;
            case VARENUM::VT_UI8:
                ConsolePrint(_T("%llu"), *(uint32_t*)pAddr);
                break;
            case VARENUM::VT_R4:
                ConsolePrint(_T("%f"), *(float*)pAddr);
                break;
            case VARENUM::VT_R8:
                ConsolePrint(_T("%lf"), *(double*)pAddr);
                break;
            case VARENUM::VT_BSTR:
                ConsolePrint(_T(R"("%s")"), WStrToTStr(*(BSTR*)pAddr).c_str());
                break;
            }

            if (index < lCount - 1)
            {
                ConsolePrint(_T(", "));
            }

            pAddr += uElemsize;
        }

        ::SafeArrayUnaccessData(pArray);
    }
}

void CWmiClassHelper::_Print(const _tstring& strName) const
{
    if (!m_pObj)
    {
        return;
    }

    HRESULT hr = S_OK;

    CComVariant vtProp;
    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, 0, 0);
    ConsolePrint(_T("%s = "), strName.c_str());
    _Print(vtProp);
}

CWmiClassHelper::CWmiClassHelper(IWbemClassObject* pObj)
{
    m_pObj = pObj;
}

wmi_boolean CWmiClassHelper::GetBoolean(const _tstring& strName, bool fTypeCheck/* = false*/) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_boolean val = 0;

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        switch (cType)
        {
        case CIMTYPE_ENUMERATION::CIM_BOOLEAN:
            val = vtProp.boolVal;
            break;
        default:
            if (!fTypeCheck)
            {
                if (0 != vtProp.ullVal)
                {
                    val = vtProp.ullVal;
                }
            }
            break;
        }
    }
    return val;
}

wmi_sint64 CWmiClassHelper::GetSInt(const _tstring& strName, bool fTypeCheck/* = false*/) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_sint64 val = 0;

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        switch (cType)
        {
        case CIMTYPE_ENUMERATION::CIM_SINT8:
        case CIMTYPE_ENUMERATION::CIM_SINT16:
        case CIMTYPE_ENUMERATION::CIM_SINT32:
        case CIMTYPE_ENUMERATION::CIM_SINT64:
            if (VARENUM::VT_BSTR == vtProp.vt)
            {
                val = _tcstoll(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
            }
            else
            {
                val = vtProp.llVal;
            }
            break;
        case CIMTYPE_ENUMERATION::CIM_UINT8:
        case CIMTYPE_ENUMERATION::CIM_UINT16:
        case CIMTYPE_ENUMERATION::CIM_UINT32:
        case CIMTYPE_ENUMERATION::CIM_UINT64:
            if (!fTypeCheck)
            {
                if (VARENUM::VT_BSTR == vtProp.vt)
                {
                    val = _tcstoull(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
                }
                else
                {
                    val = vtProp.ullVal;
                }
            }
            break;
        }
    }
    return val;
}

wmi_uint64 CWmiClassHelper::GetUInt(const _tstring& strName, bool fTypeCheck/* = false*/) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_uint64 val = 0;

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        switch (cType)
        {
        case CIMTYPE_ENUMERATION::CIM_SINT8:
        case CIMTYPE_ENUMERATION::CIM_SINT16:
        case CIMTYPE_ENUMERATION::CIM_SINT32:
        case CIMTYPE_ENUMERATION::CIM_SINT64:
            if (!fTypeCheck)
            {
                if (VARENUM::VT_BSTR == vtProp.vt)
                {
                    val = _tcstoll(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
                }
                else
                {
                    val = vtProp.llVal;
                }
            }
            break;
        case CIMTYPE_ENUMERATION::CIM_UINT8:
        case CIMTYPE_ENUMERATION::CIM_UINT16:
        case CIMTYPE_ENUMERATION::CIM_UINT32:
        case CIMTYPE_ENUMERATION::CIM_UINT64:
            if (VARENUM::VT_BSTR == vtProp.vt)
            {
                val = _tcstoull(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
            }
            else
            {
                val = vtProp.ullVal;
            }
            break;
        }
    }
    return val;
}

wmi_double CWmiClassHelper::GetFloat(const _tstring& strName, bool fTypeCheck/* = false*/) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_double val = 0.0f;

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        switch (cType)
        {
        case CIMTYPE_ENUMERATION::CIM_REAL32:
            val = vtProp.fltVal;
            break;
        case CIMTYPE_ENUMERATION::CIM_REAL64:
            val = vtProp.dblVal;
            break;
        case CIMTYPE_ENUMERATION::CIM_SINT8:
        case CIMTYPE_ENUMERATION::CIM_SINT16:
        case CIMTYPE_ENUMERATION::CIM_SINT32:
        case CIMTYPE_ENUMERATION::CIM_SINT64:
            if (!fTypeCheck)
            {
                if (VARENUM::VT_BSTR == vtProp.vt)
                {
                    val = _tcstoll(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
                }
                else
                {
                    val = vtProp.llVal;
                }
            }
            break;
        case CIMTYPE_ENUMERATION::CIM_UINT8:
        case CIMTYPE_ENUMERATION::CIM_UINT16:
        case CIMTYPE_ENUMERATION::CIM_UINT32:
        case CIMTYPE_ENUMERATION::CIM_UINT64:
            if (!fTypeCheck)
            {
                if (VARENUM::VT_BSTR == vtProp.vt)
                {
                    val = _tcstoull(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
                }
                else
                {
                    val = vtProp.ullVal;
                }
            }
            break;
        }
    }
    return val;
}

wmi_string CWmiClassHelper::GetString(const _tstring& strName, bool fTypeCheck/* = false*/) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_string val;

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        if (VARENUM::VT_BSTR == vtProp.vt)
        {
            if (fTypeCheck)
            {
                if (CIMTYPE_ENUMERATION::CIM_STRING == cType)
                {
                    val = WStrToTStr(vtProp.bstrVal);
                }
            }
            else
            {
                val = WStrToTStr(vtProp.bstrVal);
            }
        }
    }
    return val;
}

wmi_datetime CWmiClassHelper::GetDatetime(const _tstring& strName) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    wmi_datetime val = { 0 };

    if (!m_pObj)
    {
        return val;
    }

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, nullptr);
    if (SUCCEEDED(hr))
    {
        if (CIMTYPE_ENUMERATION::CIM_DATETIME == cType && VARENUM::VT_BSTR == vtProp.vt)
        {
            _stscanf_s(WStrToTStr(vtProp.bstrVal).c_str(), DATETIME_FORMAT, &val.wYear, &val.wMonth, &val.wDay, &val.wHour, &val.wMinute, &val.wSecond);
        }
    }
    return val;
}

bool CWmiClassHelper::_GetValue(
    const _tstring& strName,
    CIMTYPE ct,
    std::function<void(const CComVariant& vtProp)> cb
) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    bool fResult = false;

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        if (cb && cType == ct)
        {
            cb(vtProp);
        }

        fResult = true;
    }
    return fResult;
}

bool CWmiClassHelper::_GetValueList(
    const _tstring& strName,
    CIMTYPE ct,
    VARTYPE vt,
    std::function<void(const void* pValue, UINT uElemsize, LONG count, VARTYPE vt)> cb
) const
{
    HRESULT hr = S_OK;
    CComVariant vtProp;
    CIMTYPE cType = 0;
    bool fResult = false;

    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);

    do
    {
        if (FAILED(hr))
        {
            break;
        }

        if (((CIM_FLAG_ARRAY | ct) == cType) && ((VARENUM::VT_ARRAY | vt) == vtProp.vt) && cb)
        {
            SAFEARRAY* pArray = vtProp.parray;
            LONG lLBound = 0;
            LONG lUBound = 0;
            UINT uDim = ::SafeArrayGetDim(pArray);
            if (1 != uDim)
            {
                break;
            }

            UINT uElemsize = ::SafeArrayGetElemsize(pArray);
            ::SafeArrayGetLBound(pArray, uDim, &lLBound);
            ::SafeArrayGetUBound(pArray, uDim, &lUBound);
            LONG lCount = lUBound - lLBound;

            void* pValue = nullptr;
            ::SafeArrayAccessData(pArray, (void**)&pValue);
            cb(pValue, uElemsize, lCount, vtProp.vt);
            ::SafeArrayUnaccessData(pArray);
        }

        fResult = true;
    } while (false);

    return fResult;
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_boolean& val) const
{
    val = false;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_BOOLEAN, [&val](const CComVariant& vtProp) {
        val = vtProp.boolVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint8& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_SINT8, [&val](const CComVariant& vtProp) {
        val = vtProp.cVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint8& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_UINT8, [&val](const CComVariant& vtProp) {
        val = vtProp.bVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint16& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_SINT16, [&val](const CComVariant& vtProp) {
        val = vtProp.iVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint16& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_UINT16, [&val](const CComVariant& vtProp) {
        val = vtProp.uiVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint32& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_SINT32, [&val](const CComVariant& vtProp) {
        val = vtProp.intVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint32& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_UINT32, [&val](const CComVariant& vtProp) {
        val = vtProp.uintVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint64& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_SINT64, [&val](const CComVariant& vtProp) {
        if (VARENUM::VT_BSTR == vtProp.vt)
        {
            val = _tcstoll(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
        }
        else
        {
            val = vtProp.llVal;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint64& val) const
{
    val = 0;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_UINT64, [&val](const CComVariant& vtProp) {
        if (VARENUM::VT_BSTR == vtProp.vt)
        {
            val = _tcstoull(WStrToTStr(vtProp.bstrVal).c_str(), NULL, 10);
        }
        else
        {
            val = vtProp.ullVal;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_float& val) const
{
    val = 0.0f;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_REAL32, [&val](const CComVariant& vtProp) {
        val = vtProp.fltVal;
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_double& val) const
{
    val = 0.0f;

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_REAL64, [&val](const CComVariant& vtProp) {
        if (VARENUM::VT_R8 == vtProp.vt)
        {
            val = vtProp.dblVal;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_string& val) const
{

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_STRING, [&val](const CComVariant& vtProp) {
        if (VARENUM::VT_BSTR == vtProp.vt)
        {
            val = WStrToTStr(vtProp.bstrVal);
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_datetime& val) const
{
    val = { 0 };

    if (!m_pObj)
    {
        return false;
    }

    return _GetValue(strName, CIMTYPE_ENUMERATION::CIM_DATETIME, [&val](const CComVariant& vtProp) {
        if (VARENUM::VT_BSTR == vtProp.vt)
        {
            _stscanf_s(WStrToTStr(vtProp.bstrVal).c_str(), DATETIME_FORMAT, &val.wYear, &val.wMonth, &val.wDay, &val.wHour, &val.wMinute, &val.wSecond);
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint8_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_SINT8, VARENUM::VT_I1, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_sint8*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint8_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_UINT8, VARENUM::VT_UI1, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_uint8*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint16_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_SINT16, VARENUM::VT_I4, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_sint32*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint16_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_UINT16, VARENUM::VT_I4, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_uint32*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint32_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_SINT32, VARENUM::VT_I4, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_sint32*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint32_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_UINT32, VARENUM::VT_UI4, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_uint32*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_sint64_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_SINT64, VARENUM::VT_BSTR, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(_tcstoll(WStrToTStr((BSTR)pAddr).c_str(), NULL, 10));
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_uint64_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_UINT64, VARENUM::VT_BSTR, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(_tcstoull(WStrToTStr((BSTR)pAddr).c_str(), NULL, 10));
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_float_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_REAL32, VARENUM::VT_R4, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_float*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_double_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_REAL64, VARENUM::VT_R8, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(*(wmi_double*)pAddr);
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_string_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_STRING, VARENUM::VT_BSTR, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            valList.push_back(WStrToTStr(*(BSTR*)pAddr));
            pAddr += uElemsize;
        }
        }
    );
}

bool CWmiClassHelper::GetValue(const _tstring& strName, wmi_datetime_list& valList) const
{
    if (!m_pObj)
    {
        return false;
    }

    return _GetValueList(strName, CIMTYPE_ENUMERATION::CIM_DATETIME, VARENUM::VT_BSTR, [&valList](const void* pValue, UINT uElemsize, LONG count, VARTYPE vt) {
        size_t pAddr = (uint64_t)pValue;
        for (LONG index = 0; index < count; index++)
        {
            wmi_datetime val = { 0 };
            _stscanf_s(WStrToTStr(*(BSTR*)pAddr).c_str(), DATETIME_FORMAT, &val.wYear, &val.wMonth, &val.wDay, &val.wHour, &val.wMinute, &val.wSecond);
            valList.push_back(val);
            pAddr += uElemsize;
        }
        }
    );
}

CIMTYPE CWmiClassHelper::GetCimType(const _tstring& strName) const
{
    if (!m_pObj)
    {
        return CIMTYPE_ENUMERATION::CIM_EMPTY;
    }

    HRESULT hr = S_OK;
    VARTYPE vType = VARENUM::VT_EMPTY;
    CIMTYPE cType = CIMTYPE_ENUMERATION::CIM_EMPTY;
    CComVariant vtProp;
    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        vType = vtProp.vt;
    }
    return cType;
}

VARTYPE CWmiClassHelper::GetVarType(const _tstring& strName) const
{
    if (!m_pObj)
    {
        return VARENUM::VT_EMPTY;
    }

    HRESULT hr = S_OK;
    VARTYPE vType = VARENUM::VT_EMPTY;
    CIMTYPE cType = CIMTYPE_ENUMERATION::CIM_EMPTY;
    CComVariant vtProp;
    hr = m_pObj->Get(TStrToWStr(strName).c_str(), 0, &vtProp, &cType, 0);
    if (SUCCEEDED(hr))
    {
        vType = vtProp.vt;
    }
    return vType;
}

wmi_string CWmiClassHelper::GetClass() const
{
    return GetString(_T("__CLASS"));
}

wmi_string CWmiClassHelper::GetSuperClass() const
{
    return GetString(_T("__SUPERCLASS"));
}

wmi_string CWmiClassHelper::GetNameSpace() const
{
    return GetString(_T("__NAMESPACE"));
}

wmi_string CWmiClassHelper::GetPath() const
{
    return GetString(_T("__PATH"));
}

wmi_string CWmiClassHelper::GetServer() const
{
    return GetString(_T("__SERVER"));
}

wmi_string CWmiClassHelper::GetRelPath() const
{
    return GetString(_T("__RELPATH"));
}

wmi_sint32 CWmiClassHelper::GetPropertyCount() const
{
    return GetSInt(_T("__PROPERTY_COUNT"));
}

wmi_sint32 CWmiClassHelper::GetGenus() const
{
    return GetSInt(_T("__GENUS"));
}

wmi_string CWmiClassHelper::GetDynasty() const
{
    return GetString(_T("__DYNASTY"));
}

wmi_string CWmiClassHelper::GetDerivation() const
{
    return GetString(_T("__DERIVATION"));
}

void CWmiClassHelper::PrintPropertySystemOnly() const
{
    _PrintProperty(WBEM_FLAG_ALWAYS | WBEM_FLAG_SYSTEM_ONLY);
}

void CWmiClassHelper::PrintPropertyNonSystemOnly() const
{
    _PrintProperty(WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY);
}

void CWmiClassHelper::PrintProperty() const
{
    _PrintProperty(WBEM_FLAG_ALWAYS);
}

CWmiHelper::CWmiHelper() :
    m_pSvc(NULL),
    m_pLoc(NULL),
    m_bInit(false)
{

}

CWmiHelper::~CWmiHelper()
{
    Uninitialize();
}

bool CWmiHelper::Initialize(_tstring strNamespace/* = _T(R"(ROOT\CIMV2)")*/)
{
    HRESULT hr = S_OK;

    if (m_bInit)
    {
        return true;
    }

    hr = ::CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return false;
    }

    do
    {
        hr = CoInitializeSecurity(
            NULL,
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            NULL                         // Reserved
        );

        hr = ::CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&m_pLoc);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pLoc->ConnectServer(
            _bstr_t(TStrToWStr(strNamespace).c_str()),    // Object path of WMI namespace
            NULL,                       // User name. NULL = current user
            NULL,                       // User password. NULL = current
            0,                          // Locale. NULL indicates current
            NULL,                       // Security flags.
            0,                          // Authority (for example, Kerberos)
            0,                          // Context object 
            &m_pSvc                     // pointer to IWbemServices proxy
        );
        if (FAILED(hr))
        {
            break;
        }

        hr = ::CoSetProxyBlanket(
            m_pSvc,                      // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            NULL,                        // Server principal name 
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            NULL,                        // client identity
            EOAC_NONE                    // proxy capabilities 
        );
        if (FAILED(hr))
        {
            break;
        }

        m_bInit = true;

    } while (false);

    if (!m_bInit)
    {
        if (m_pSvc)
        {
            m_pSvc->Release();
            m_pSvc = NULL;
        }

        if (m_pLoc)
        {
            m_pLoc->Release();
            m_pLoc = NULL;
        }

        ::CoUninitialize();
    }

    return m_bInit;
}

void CWmiHelper::Uninitialize()
{
    if (m_bInit)
    {
        if (m_pSvc)
        {
            m_pSvc->Release();
            m_pSvc = NULL;
        }

        if (m_pLoc)
        {
            m_pLoc->Release();
            m_pLoc = NULL;
        }

        ::CoUninitialize();

        m_bInit = false;
    }
}

bool CWmiHelper::Query(
    const _tstring& strWql,
    std::function<void(const CWmiClassHelper& obj)> cb/* = nullptr*/
) const
{
    HRESULT hr = S_OK;
    IEnumWbemClassObject* pEnumerator = NULL;

    if (!m_bInit)
    {
        return false;
    }

    hr = m_pSvc->ExecQuery(
        bstr_t(_T("WQL")),
        bstr_t(strWql.c_str()),
        WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
        NULL,
        &pEnumerator);

    if (FAILED(hr))
    {
        return false;
    }

    IWbemClassObject* pObj = NULL;
    ULONG uReturn = 0;

    while (true)
    {
        if (FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn)))
        {
            break;
        }

        if (0 == uReturn)
        {
            break;
        }

        CWmiClassHelper classObj(pObj);
        if (cb)
        {
            cb(classObj);
        }

        pObj->Release();
    }

    pEnumerator->Release();
    pEnumerator = NULL;

    return true;
}

bool CWmiHelper::QueryClass(
    const _tstring& strClass,
    std::function<void(const CWmiClassHelper& obj)> cb/* = nullptr*/
) const
{
    wmi_string strSql = _T("SELECT * FROM ") + strClass;
    return Query(strSql, cb);
}

bool CWmiHelper::QueryClassWithNames(
    const _tstring& strClass,
    std::set<wmi_string> names/* = {}*/,
    std::function<void(const CWmiClassHelper& obj)> cb/* = nullptr*/
) const
{
    wmi_string strSql = _T("SELECT * FROM ") + strClass;
    if (!names.empty())
    {
        strSql = _T("SELECT ");

        size_t count = names.size();
        for (const auto& item : names)
        {
            strSql += item;
            count--;
            if (count)
            {
                strSql += _T(", ");
            }
        }

        strSql += _T(" FROM ");
        strSql += strClass;
    }

    return Query(strSql, cb);
}

bool CWmiHelper::QueryClassWithoutNames(
    const _tstring& strClass,
    std::set<wmi_string> names/* = {}*/,
    std::function<void(const CWmiClassHelper& obj)> cb/* = nullptr*/
) const
{
    wmi_string strSql = _T("SELECT * FROM ") + strClass;
    if (!names.empty())
    {
        std::set<wmi_string> nameList = QueryClassNames(strClass);
        for (const auto& item : names)
        {
            auto itFind = nameList.find(item);
            if (itFind != nameList.end())
            {
                nameList.erase(itFind);
            }
        }

        // 没有查询任何字段
        if (nameList.empty())
        {
            return false;
        }

        strSql = _T("SELECT ");
        size_t count = nameList.size();
        for (const auto& item : nameList)
        {
            strSql += item;
            count--;
            if (count)
            {
                strSql += _T(", ");
            }
        }

        strSql += _T(" FROM ");
        strSql += strClass;
    }

    return Query(strSql, cb);
}

std::set<wmi_string> CWmiHelper::QueryClassNames(const _tstring& strClass) const
{
    std::set<wmi_string> result;
    HRESULT hr = S_OK;

    IWbemClassObject* pClass = NULL;

    do
    {
        if (!m_bInit)
        {
            break;
        }

        hr = m_pSvc->GetObject(_bstr_t(strClass.c_str()), 0, NULL, &pClass, NULL);
        if (FAILED(hr))
        {
            break;
        }

        SAFEARRAY* pNames = NULL;
        hr = pClass->GetNames(NULL, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, NULL, &pNames);
        if (FAILED(hr))
        {
            break;
        }

        LONG lLBound = 0;
        LONG lUBound = 0;
        UINT uDim = ::SafeArrayGetDim(pNames);
        ::SafeArrayGetLBound(pNames, uDim, &lLBound);
        ::SafeArrayGetUBound(pNames, uDim, &lUBound);

        BSTR* pValue;
        ::SafeArrayAccessData(pNames, (void**)&pValue);
        for (int i = 0; i <= lUBound - lLBound; i++)
        {
            result.insert(WStrToTStr(*pValue).c_str());
            pValue++;
        }
        ::SafeArrayUnaccessData(pNames);
        ::SafeArrayDestroy(pNames);
    } while (false);

    if (pClass)
    {
        pClass->Release();
    }

    return result;
}