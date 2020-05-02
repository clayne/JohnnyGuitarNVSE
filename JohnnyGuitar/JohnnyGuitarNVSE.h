#include "..\..\nvse\nvse\ScriptUtils.h"
#include "internal/decoding.h"
#include <Windows.h>
#pragma once

NVSEArrayVarInterface *ArrIfc = NULL;
NVSEStringVarInterface *StrIfc = NULL;
NVSEMessagingInterface *g_msg = NULL;
NVSEScriptInterface *g_script = NULL;
NVSECommandTableInterface *CmdIfc = NULL; 
InventoryRef* (*InventoryRefGetForID)(UInt32 refID);
float(*GetWeaponDPS)(ActorValueOwner *avOwner, TESObjectWEAP *weapon, float condition, UInt8 arg4, ContChangesEntry *entry, UInt8 arg6, UInt8 arg7, int arg8, float arg9, float arg10, UInt8 arg11, UInt8 arg12, TESForm *ammo) =
(float(*)(ActorValueOwner*, TESObjectWEAP*, float, UInt8, ContChangesEntry*, UInt8, UInt8, int, float, float, UInt8, UInt8, TESForm*))0x645380;
bool isShowLevelUp = true;
char* StrArgBuf;
#define ExtractArgsEx(...) g_script->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_script->ExtractFormatStringArgs(__VA_ARGS__)
#define IS_TYPE(form, type) (*(UInt32*)form == kVtbl_##type)
#define NOT_ID(form, type) (form->typeID != kFormType_##type)
#define IS_ID(form, type) (form->typeID == kFormType_##type)
#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);
#define REG_TYPED_CMD(name, type) nvse->RegisterTypedCommand(&kCommandInfo_##name,kRetnType_##type);
IDebugLog ParamLog;
bool loadEditorIDs = 0;
bool fixHighNoon = 0;

__declspec(naked) bool __fastcall HasSeenData(TESObjectCELL *cell) {
	__asm {
		push	kExtraData_SeenData
		add		ecx, 0x28
		call	BaseExtraList::GetByType
		test	eax, eax
		setnz	al
		retn
	}
}
__declspec(naked) SInt32 __fastcall GetDetachTime(TESObjectCELL *cell) {
	__asm {
		push	kExtraData_DetachTime
		add		ecx, 0x28
		call	BaseExtraList::GetByType
		test	eax, eax
		jz done
		mov eax, [eax+0xC]
		done:
			retn
	}
}
__declspec(naked) void Tile::SetFloat(UInt32 id, float fltVal, bool bPropagate)
{
	static const UInt32 procAddr = 0xA012D0;
	__asm	jmp		procAddr
}

__declspec(naked) float ExtraContainerChanges::EntryData::GetItemHealthPerc(bool arg1)
{
	static const UInt32 procAddr = 0x4BCDB0;
	__asm	jmp		procAddr
}
__declspec(naked) ContChangesEntry *ExtraContainerChanges::EntryDataList::FindForItem(TESForm *item)
{
	__asm
	{
		mov		edx, [esp + 4]
		listIter:
		mov		eax, [ecx]
			test	eax, eax
			jz		listNext
			cmp[eax + 8], edx
			jz		done
			listNext :
		mov		ecx, [ecx + 4]
			test	ecx, ecx
			jnz		listIter
			xor		eax, eax
			done :
		retn	4
	}
}

__declspec(naked) ExtraContainerChanges::EntryDataList *TESObjectREFR::GetContainerChangesList()
{
	__asm
	{
		push	kExtraData_ContainerChanges
		add		ecx, 0x44
		call	BaseExtraList::GetByType
		test	eax, eax
		jz		done
		mov		eax, [eax + 0xC]
		test	eax, eax
		jz		done
		mov		eax, [eax]
		done:
		retn
	}
}
UInt8 TESForm::GetOverridingModIdx()
{
	ModInfo *info = mods.GetLastItem();
	return info ? info->modIndex : 0xFF;
}
_declspec(naked) void LevelUpHook() {
	static const UInt32 noShowAddr = 0x77D903;
	static const UInt32 showAddr = 0x77D618;
	_asm {
		jne noLevelUp
		cmp dword ptr ds : [isShowLevelUp], 0
		je noLevelUp
		jmp showAddr
		noLevelUp:
		jmp noShowAddr
	}
}

__declspec(naked) void GetNameHook() {
	__asm jmp TESForm::hk_GetName
}
__declspec(naked) void SetEditorIdHook() {
	__asm jmp TESForm::hk_SetEditorId
}
__declspec(naked) void SetEditorIdHook_REFR()
{	__asm jmp TESForm::hk_SetEditorID_REFR
}
const char* __fastcall ConsoleNameHook(TESObjectREFR* ref) {
	const char* name = ref->baseForm->GetTheName();
	if (!strlen(name)) name = ref->baseForm->GetName();
	return name;
}
void LoadEditorIDs() {
	WriteRelCall(0x486903, (UInt32(GetNameHook))); // replaces empty string with editor id in TESForm::GetDebugName
	WriteRelCall(0x71B748, UInt32(ConsoleNameHook)); // replaces empty string with editor id in selected ref name in console
	WriteRelCall(0x710BFC, UInt32(ConsoleNameHook));
	WriteRelCall(0x55D498, (UInt32(GetNameHook))); // replaces empty string with editor id in TESObjectREFR::GetDebugName
	WriteRelJump(0x467A15, 0x467A4E); // loads more types in game's editor:form map
	for (uint32_t i = 0; i < ARRAYSIZE(TESForm_Vtables); i++)
	{
		if (*(uintptr_t*)(TESForm_Vtables[i] + 0x130) == 0x00401280)
			SafeWrite32(TESForm_Vtables[i] + 0x130, (UInt32)GetNameHook);

		if (*(uintptr_t*)(TESForm_Vtables[i] + 0x134) == 0x00401290)
			SafeWrite32(TESForm_Vtables[i] + 0x134, (UInt32)SetEditorIdHook);
	}
	for (uint32_t i = 0; i < ARRAYSIZE(TESForm_REFR_Vtables); i++)
	{		if (*(uintptr_t*)(TESForm_REFR_Vtables[i] + 0x130) == 0x00401280)
			SafeWrite32(TESForm_REFR_Vtables[i] + 0x130, (UInt32)GetNameHook);

		if (*(uintptr_t*)(TESForm_REFR_Vtables[i] + 0x134) == 0x00401290)
			SafeWrite32(TESForm_REFR_Vtables[i] + 0x134, (UInt32)SetEditorIdHook_REFR);
	}
}
bool __fastcall CheckForHighNoon(Sky* sky)
{
	return (float)(sky->firstClimate->sunriseEnd) / 6.0 < sky->gameHour && sky->gameHour < (float)(sky->firstClimate->sunsetBegin) / 6.0;
}

__declspec (naked) void HookforIMOD1()
{
	static const UInt32 retaddress = 0x063F575;
	__asm {
		mov ecx, dword ptr ds : [0x11DEA20]
		call CheckForHighNoon
		test al, al
		mov ecx, [ebp - 0x7C]
		jz HandleNormal
		mov ecx, [ecx + 0x120]
		jmp RETURN
		HandleNormal :
		mov ecx, [ecx + 0x11C]
			RETURN :
			jmp retaddress
	}
}

__declspec (naked) void HookforIMOD2()
{
	static const UInt32 retaddress = 0x063F5F6;

	__asm {
		mov ecx, dword ptr ds : [0x11DEA20]
		call CheckForHighNoon
		test al, al
		mov ecx, [ebp - 0x7C]
		jz HandleNormal
		mov ecx, [ecx + 0x11C]
		jmp RETURN
		HandleNormal :
		mov ecx, [ecx + 0x120]
			RETURN :
			jmp retaddress
	}
}

void HandleGameHooks()
{
	WriteRelJump(0xC5244A, (UInt32)NiCameraGetAltHook);
	WriteRelJump(0x77D612, UInt32(LevelUpHook));
	if (loadEditorIDs) LoadEditorIDs();
	if (fixHighNoon) {
		WriteRelJump((UInt32)0x063F56C, (UInt32)HookforIMOD1);
		WriteRelJump((UInt32)0x063F5ED, (UInt32)HookforIMOD2);
	}
}
static void PatchMemoryNop(ULONG_PTR Address, SIZE_T Size)
{
	DWORD d = 0;
	VirtualProtect((LPVOID)Address, Size, PAGE_EXECUTE_READWRITE, &d);

	for (SIZE_T i = 0; i < Size; i++)
		*(volatile BYTE *)(Address + i) = 0x90; //0x90 == opcode for NOP

	VirtualProtect((LPVOID)Address, Size, d, &d);

	FlushInstructionCache(GetCurrentProcess(), (LPVOID)Address, Size);
}

bool removeFiles(char* folder1)
{
	char folder[MAX_PATH];
	char filename[MAX_PATH];
	strcpy(folder, folder1);
	strcat(folder, "/*.*");
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(folder, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				sprintf(filename, "%s\\%s", folder1, fd.cFileName);
				remove(filename);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
		RemoveDirectory(folder1);
	}
	return 1;
}

TESRegionDataWeather* GetWeatherData(TESRegion* region) {
	ListNode<TESRegionData> *iter = region->dataEntries->Head();
	TESRegionData *regData;
	do
	{
		regData = iter->data;
		if ((*(UInt32*)regData == 0x1023E18))
			return (TESRegionDataWeather*)regData;
	} while (iter = iter->next);
	return NULL;
}