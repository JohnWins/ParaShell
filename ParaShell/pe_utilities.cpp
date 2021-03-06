#include <cstdlib>
#include <windows.h>
#include "pe_utilities.h"
#include "error.h"

/*
	Description:	取整对齐函数
*/
UINT AlignSize(UINT nSize, UINT nAlign)
{
	return ((nSize + nAlign - 1) / nAlign * nAlign);
}


/* 
	Description:	RVA->指向堆中对应位置的指针						   
*/
char* RVAToPtr(const void* imagebase, const DWORD dwRVA)
{
	return ((char*)imagebase + dwRVA);
}


/*
	Description:	获取NT头指针
*/
PIMAGE_NT_HEADERS getNTHeader(const void* imagebase)
{
	return (PIMAGE_NT_HEADERS)((char*)imagebase + ((PIMAGE_DOS_HEADER)imagebase)->e_lfanew);
}

/*
	Description:	获取section表指针
*/
PIMAGE_SECTION_HEADER getSecHeader(const void* _imagebase)
{
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)getNTHeader(_imagebase);

	return (PIMAGE_SECTION_HEADER)((char*)pNTHeaders + sizeof(IMAGE_NT_HEADERS));
	
}


/*
	Description:	获取最后一个区块表项指针
*/
PIMAGE_SECTION_HEADER getLastSecHeader(const void* _pImageBase)
{
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)getNTHeader(_pImageBase);
	PIMAGE_SECTION_HEADER pSecHeader = getSecHeader(_pImageBase);

	pSecHeader += pNTHeaders->FileHeader.NumberOfSections - 1;

	return pSecHeader;
}


/*
	Description:	搜索并去掉尾部无用的零字节，重新计算区块的大小             
*/
unsigned int CalcMinSizeOfData(char* pSectionData, const unsigned int nSectionSize)
{

	if (IsBadReadPtr(pSectionData, nSectionSize))
	{
		return nSectionSize;
	}

	char*	pData = pSectionData + nSectionSize - 1;
	unsigned int	nSize = nSectionSize;

	while (nSize > 0 && *pData == 0)
	{
		pData--;
		nSize--;
	}

	return nSize;
}


const int nListNum = 6;
const char* szSecNameList[nListNum] =
{
	".text",
	".data",
	".rdata",
	"CODE",
	"DATA",
	".reloc"
};
/*
	Description:	判断当前区块数据能否被压缩
*/
bool IsSectionPackable(PIMAGE_SECTION_HEADER pSecHeader)
{
	// 如果发现匹配的区块名称，则表示此区块可以压缩
	for (UINT nIndex = 0; nIndex < nListNum; nIndex++)
	{

		/*有些输出表可能会在.rdata等区块，如果区块合并了就不能这样判断了
		if (!IsMergeSection)
		{
			if ((nExportAddress >= pSecHeader->VirtualAddress) && (nExportAddress < (pSecHeader->VirtualAddress + pSecHeader->Misc.VirtualSize)))
				return FALSE;
		}
		*/

		if (strncmp((char *)pSecHeader->Name, szSecNameList[nIndex], strlen(szSecNameList[nIndex])) == 0)
		{
			return true;
		}
	}

	return false;
}


/*
	Description:	备份文件
*/
int BackUpFile(TCHAR *szFilePath)
{
	TCHAR *szFilebakName = new TCHAR[MAX_PATH * sizeof(TCHAR)];
	
	ZeroMemory(szFilebakName, MAX_PATH * sizeof(TCHAR));

	lstrcpy(szFilebakName, szFilePath);
	lstrcat(szFilebakName, TEXT(".bak"));
	CopyFile(szFilePath, szFilebakName, FALSE);

	delete []szFilebakName;

	return ERR_SUCCESS;
}

/*
	Description:	获取DOS头大小
*/
unsigned int GetDosHeaderSize(void* _pImageBase)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)_pImageBase;

	return pDosHeader->e_lfanew;
}


/*
	Description:	获取NT头大小
*/
unsigned int GetNTHeaderSize(void* _pImageBase)
{
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)getNTHeader(_pImageBase);

	unsigned int NTHeaderSize = sizeof(pNTHeader->Signature) + sizeof(pNTHeader->FileHeader) + pNTHeader->FileHeader.SizeOfOptionalHeader;

	return NTHeaderSize;
}


/*
	Description:	获取区块表大小
*/
unsigned int GetSectionTableSize(void* _pImageBase)
{
	// TODO
	return ERR_SUCCESS;
}


/*
	Description:	在区块表最后添加新区快,new申请新区快内存，需要调用者delete
*/
unsigned int CreateNewSection(void* _pImageBase, const DWORD _secsize, void **_ppNewSection)
{
	PIMAGE_NT_HEADERS pNTHeader = getNTHeader(_pImageBase);
	PIMAGE_SECTION_HEADER pNewSecHeader = getLastSecHeader(_pImageBase) + 1;
	PIMAGE_SECTION_HEADER pLastValidSecHeader = getLastSecHeader(_pImageBase);

	/*  把所有区块往后移动  */
	/* 从最后一个区块开始，向后一个区块移动*/
	/*
	for (int i = pNTHeader->FileHeader.NumberOfSections; i > 0; i--, pLastValidSecHeader--)
	{
		memcpy(pLastValidSecHeader + 1, pLastValidSecHeader, sizeof(IMAGE_SECTION_HEADER));
	}
	*/

	/* 
		有些文件的区块是空区块。如.textbbs 
		需要挪到最后一个有效的区块
	*/
	while (0 == pLastValidSecHeader->PointerToRawData)
	{
		pLastValidSecHeader--;
	}

	/*  填写新区块信息  */
	memset(pNewSecHeader, 0, sizeof(IMAGE_SECTION_HEADER));
	/* Name, VirtualAddress, VirtualSize, RawAddress, RawSize, Characteristics */
	const char newsecname[8] = { ".shell" };
	memcpy(pNewSecHeader->Name, newsecname, 8);
	pNewSecHeader->VirtualAddress = pLastValidSecHeader->VirtualAddress + AlignSize(pLastValidSecHeader->Misc.VirtualSize, pNTHeader->OptionalHeader.SectionAlignment);
	pNewSecHeader->Misc.VirtualSize = AlignSize(_secsize, pNTHeader->OptionalHeader.SectionAlignment);
	pNewSecHeader->PointerToRawData = pLastValidSecHeader->PointerToRawData + AlignSize(pLastValidSecHeader->SizeOfRawData, pNTHeader->OptionalHeader.FileAlignment);
	pNewSecHeader->SizeOfRawData = AlignSize(_secsize, pNTHeader->OptionalHeader.FileAlignment);
	pNewSecHeader->Characteristics = 0xE0000020;


	/*  分配新区块内存  */
	DWORD ulNewSecSize = AlignSize(_secsize, pNTHeader->OptionalHeader.SectionAlignment);
	void* ptr = new char[ulNewSecSize];
	*_ppNewSection = ptr;
	memset(*_ppNewSection, 0, ulNewSecSize);


	/*  修复PE头相关项  */
	/* SizeOfImage, NumberOfSections, SizeOfCode */
	pNTHeader->OptionalHeader.SizeOfImage = AlignSize(pNTHeader->OptionalHeader.SizeOfImage + ulNewSecSize, pNTHeader->OptionalHeader.SectionAlignment);
	pNTHeader->FileHeader.NumberOfSections++;
	pNTHeader->OptionalHeader.SizeOfCode += ulNewSecSize;

	return ERR_SUCCESS;
}


/*
	Description:	把输入的内存块融合到一起
*/
void* MergeMemBlock(void* _pImageBase, void* _pShellSection)
{
	PIMAGE_NT_HEADERS pNTHeader = getNTHeader(_pImageBase);
	PIMAGE_SECTION_HEADER pShellSecHeader = getLastSecHeader(_pImageBase);
	DWORD ulNewImageSize = pNTHeader->OptionalHeader.SizeOfImage;
	DWORD ulOriginalImageSize = ulNewImageSize - AlignSize(pShellSecHeader->Misc.VirtualSize, pNTHeader->OptionalHeader.SectionAlignment);
	DWORD ulShellSize = pShellSecHeader->SizeOfRawData;

	// 分配新映像的内存空间
	void* pNewMemBlock = new unsigned char[ulNewImageSize];
	memset(pNewMemBlock, 0, ulNewImageSize);

	// 复制原ImageBase
	memcpy(pNewMemBlock, _pImageBase, ulOriginalImageSize);

	// 复制ShellSection
	void* pNewShellPosition = (void*)((DWORD)pNewMemBlock + ulOriginalImageSize);
	memcpy(pNewShellPosition, _pShellSection, ulShellSize);

	return pNewMemBlock;
}

/*
	Description:	把指定区块属性设为可写
*/
bool MakeSecWritable(void *_pImageBase, DWORD Offset)
{
	if (!_pImageBase || !Offset)
	{
		return false;
	}

	PIMAGE_NT_HEADERS pNTHeader = getNTHeader(_pImageBase);
	PIMAGE_SECTION_HEADER pSecHeader = getSecHeader(_pImageBase);

	while (!(
		Offset >= pSecHeader->VirtualAddress \
		&& Offset <= (pSecHeader->VirtualAddress + pSecHeader->Misc.VirtualSize)))
	{
		pSecHeader++;
	}
	if (Offset >= pSecHeader->VirtualAddress \
		&& Offset <= (pSecHeader->VirtualAddress + pSecHeader->Misc.VirtualSize))
	{
		pSecHeader->Characteristics |= IMAGE_SCN_MEM_WRITE;
	}

	return true;
}