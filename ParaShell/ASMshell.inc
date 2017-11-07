IFNDEF __ASMSHELL_INC__
__ASMSHELL_INC__	equ	<1>
;__ASMSHELL_DEBUG__	equ <1>

PUBLIC  C	Label_Shell_Start
PUBLIC  C   Label_Shell_End
PUBLIC	C	Label_Induction_Start
PUBLIC	C	Label_Induction_End
PUBLIC	C	Label_Induction_Data_Start
PUBLIC  C   Label_Induction_Data_End
PUBLIC  C 	Label_Induction_Import_Start
PUBLIC  C 	Label_Induction_Import_End
PUBLIC	C	Label_Luanch_Start
PUBLIC	C	Label_Luanch_End
PUBLIC	C	Lable_Luanch_Data_Start
PUBLIC 	C 	Lable_Luanch_Data_End
;PUBLIC	C	TlsTable
;PUBLIC	C	RelocBaseBegin


MY_IMAGE_IMPORT_DESCRIPTOR STRUCT    
	
	UNION Misc
		Characteristics 	DD	?	; 0 for terminating null import descriptor
		OriginalFirstThunk 	DD	?	; RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	ENDS
	
	TimeDateStamp	DD	?
	
    ForwarderChain	DD	?
    
	DLLName			DD	?
    
	FirstThunk		DD	?            ; RVA to IAT
	
MY_IMAGE_IMPORT_DESCRIPTOR ENDS

MY_IMAGE_IMPORT_THUNK STRUCT
	UNION u1
		Ordinal		DWORD	?
		FuncAddr	DWORD	?
	ENDS
MY_IMAGE_IMPORT_THUNK ENDS


MY_IMAGE_IMPORT_BY_NAME STRUCT
	Hint		DW	?
	FuncName	DB	32 DUP (?)
MY_IMAGE_IMPORT_BY_NAME ENDS


INDUCTION_DATA STRUCT
	
	nShellStep	 		DD	0	; DLL���ܻ��ν���shell,ͨ��ͳ�ƽ��벽�����ж���EXE����DLL
	
	LuanchBase	 		DD	?	; ��Ҫ��ǳ���д��	RVA
	
	LuanchAllocBase	 	DD	?
	
	PresentImageBase	DD	?
	
	nLuanchOriginalSize	DD	?
	
	nLuanchPackSize		DD	?	; ��Ҫ��ǳ���д��
	
	szVirtualAlloc	 	DB	'VirtualAlloc', 0
	
	VirtualAllocAddr 	DD	0
	
	TlsTable	 		DB	18h DUP (?)	; ��ʱδʹ�õ�

INDUCTION_DATA ENDS


LUANCH_DATA STRUCT
OEP					DD	?	; ��Ҫ��ǳ���д��
IsMutateImpTable	DD	?	; ��Ҫ��ǳ���д��
MutateImpTableAddr	DD	?	; ��Ҫ��ǳ���д��
OriginalImpTableAddr	DD	?	; ��Ҫ��ǳ���д��
IsDll				DD	?	; ��Ҫ��ǳ���д��
OriginalRelocAddr	DD	?	; ��Ҫ��ǳ���д��	
SectionPackInfo		DB	0a0h DUP (?)	; ��Ҫ��ǳ���д��
nLuanchAllocStep    DD	0
szVirtualFree		DB	'VirtualFree', 0
VirtualFreeADDR		DD	0
szKer32DLLName		DB	'KERNEL32.dll', 0, 0
hFile				DD	0
; ����������Ҫ�ӵ�һ��SHELL���Ƶ��ڶ���SHELL
; �����������򲻿ɸı�
GPAAddr				DD	0
GMHAddr				DD	0
LLAAddr				DD	0
VirtualAllocAddr	DD	0
aPDepackASMAddr		DD	0
PresentImageBase	DD	0
LUANCH_DATA ENDS


comment /
	FirstThunk|DLLName[32]|nFunc|FuncName[32]...
/
MUTATE_IMPORT_THUNK UNION
	Ordinal		DWORD	?
	ProcName	BYTE	32 DUP (?)
MUTATE_IMPORT_THUNK ENDS

MUTATE_IMPORT	STRUCT
	FirstThunk	DD	?
	DLLName		DB	32 DUP (?)
	nFunc		DD	?
	FuncName	MUTATE_IMPORT_THUNK <?>
MUTATE_IMPORT 	ENDS


comment /
	Description:
		C convention
		unsigned int aP_depack_safe(const void *source,
                            unsigned int srclen,
                            void *destination,
                            unsigned int dstlen);
/
Proc_aP_depack_asm_safe PROTO


comment /
	Description:	
	Parameters:	_RuntimeImageBase		DWORD
				_MutateImportRVA		DWORD	RVA to ImageBase
				_GPAAddr				DWORD
				_GMHAddr				DWORD
				_LLAAddr				DWORD
/
Proc_UnmutateImport	PROTO C , _RuntimeImageBase:DWORD, _MutateImportRVA:DWORD, _GPAAddr:DWORD, _GMHAddr:DWORD, _LLAAddr:DWORD


comment /
	Description:	��ʼ��ԭ�����
					C convention
	Parameters:		_RuntimeImageBase	DWORD
					_OriginalImportRVA	DWORD	RVA to ImageBase
					_GPAAddr			DWORD	
					_GMHAddr			DWORD
					_LLAAddr			DWORD
/
Proc_InitOrigianlImport PROTO C , _RuntimeImageBase:DWORD, _OriginalImportRVA:DWORD, _GPAAddr:DWORD, _GMHAddr:DWORD, _LLAAddr:DWORD


ENDIF  ; __ASMSHELL_INC__