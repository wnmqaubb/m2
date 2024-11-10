

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Oct 30 16:10:30 2024
 */
/* Compiler settings for GateF.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __Gate_F_h_h__
#define __Gate_F_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGateF_FWD_DEFINED__
#define __IGateF_FWD_DEFINED__
typedef interface IGateF IGateF;
#endif 	/* __IGateF_FWD_DEFINED__ */


#ifndef __GateF_FWD_DEFINED__
#define __GateF_FWD_DEFINED__

#ifdef __cplusplus
typedef class GateF GateF;
#else
typedef struct GateF GateF;
#endif /* __cplusplus */

#endif 	/* __GateF_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __GateF_LIBRARY_DEFINED__
#define __GateF_LIBRARY_DEFINED__

/* library GateF */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_GateF;

#ifndef __IGateF_DISPINTERFACE_DEFINED__
#define __IGateF_DISPINTERFACE_DEFINED__

/* dispinterface IGateF */
/* [uuid] */ 


EXTERN_C const IID DIID_IGateF;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("12ab5939-59df-497e-aab9-9fc17412dd88")
    IGateF : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IGateFVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGateF * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGateF * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGateF * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGateF * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGateF * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGateF * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGateF * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IGateFVtbl;

    interface IGateF
    {
        CONST_VTBL struct IGateFVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGateF_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGateF_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGateF_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGateF_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGateF_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGateF_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGateF_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IGateF_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_GateF;

#ifdef __cplusplus

class DECLSPEC_UUID("42dd2538-7123-4594-9d5e-adb22f860935")
GateF;
#endif
#endif /* __GateF_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


