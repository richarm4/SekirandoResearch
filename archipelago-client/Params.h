// Much of this comes from the TGA table
// https://github.com/The-Grand-Archives/Dark-Souls-III-CT-TGA/blob/main/resources/include/tga

#pragma once
#include "fd4_singleton.h"

typedef struct _DLWString {
	union {
		wchar_t in_place[8];
		wchar_t* ptr;
	} str;
	uint64_t length;
	uint64_t capacity;
} DLWString;

inline wchar_t* dlw_c_str(DLWString* s) {
	return (s->capacity > 7) ? s->str.ptr : s->str.in_place;
}

typedef struct _ParamRowInfo {
	uint64_t row_id; // ID of param row
	uint64_t param_offset; // Offset of pointer to param data relative to parent table
	uint64_t param_end_offset; // Seems to point to end of ParamTable struct
} ParamRowInfo;

typedef struct _ParamTable {
	uint8_t pad00[0x00A];
	uint16_t num_rows; // Number of rows in param table

	uint8_t pad01[0x004];
	uint64_t param_type_offset; // Offset of param type string from the beginning of this struct

	uint8_t pad02[0x028];
	ParamRowInfo rows[0]; // Array of row information structs
} ParamTable;

typedef struct _ParamHeader {
	uint8_t pad00[0x60];
	uint64_t param_table_size_bytes;
	ParamTable* param_table;
} ParamHeader;

typedef struct _ParamResCap {
	void** vftable_ptr;

	uint8_t pad00[0x08];
	DLWString param_name;

	uint8_t pad01[0x38];
	ParamHeader* param_header;
} ParamResCap;

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef char fixstr;
typedef short fixstrW;
typedef float f32;
typedef double f64;
typedef unsigned char dummy8;

typedef struct _EquipParamAccessoryRow {
	s32 refId;
	s32 sfxVariationId;
	f32 weight;
	s32 behaviorId;
	s32 basicPrice;
	s32 sellValue;
	s32 sortId;
	s32 qwcId;
	s16 equipModelId;
	s16 iconId;
	s16 shopLv;
	s16 trophySGradeId;
	s16 trophySeqId;
	u8 equipModelCategory;
	u8 equipModelGender;
	u8 accessoryCategory;
	u8 refCategory;
	u8 spEffectCategory;
	dummy8 pad1[1];
	s32 vagrantItemLotId;
	s32 vagrantBonusEneDropItemLotId;
	s32 vagrantItemEneDropItemLotId;
	u8 IsDeposit : 1;
	u8 IsEquipOutBrake : 1;
	u8 disableMultiDropShare : 1;
	u8 IsDiscard : 1;
	u8 isDrop : 1;
	u8 showLogCondType : 1;
	u8 showDialogCondType : 2;
	dummy8 pad2[3];
	s32 saleValue;
	s16 accessoryGroup;
	u8 vowId;
	u8 compTrophySedId;
	s32 residentSpEffectId1;
	s32 residentSpEffectId2;
	s32 residentSpEffectId3;
	s32 residentSpEffectId4;
	dummy8 pad3[8];
} EquipParamAccessoryRow;

typedef struct _EquipParamGoodsRow {
	s32 refId_default;
	s32 sfxVariationId;
	f32 weight;
	s32 basicPrice;
	s32 sellValue;
	s32 behaviorId;
	s32 replaceItemId;
	s32 sortId;
	s32 appearanceReplaceItemId;
	s32 yesNoDialogMessageId;
	s32 magicId;
	s16 iconId;
	s16 modelId;
	s16 shopLv;
	s16 compTrophySedId;
	s16 trophySeqId;
	s16 maxNum;
	u8 consumeHeroPoint;
	u8 overDexterity;
	u8 goodsType;
	u8 refCategory;
	u8 spEffectCategory;
	u8 pad3;
	s8 goodsUseAnim;
	u8 opmeMenuType;
	u8 useLimitCategory;
	u8 replaceCategory;
	u8 vowType0 : 1;
	u8 vowType1 : 1;
	u8 vowType2 : 1;
	u8 vowType3 : 1;
	u8 vowType4 : 1;
	u8 vowType5 : 1;
	u8 vowType6 : 1;
	u8 vowType7 : 1;
	u8 vowType8 : 1;
	u8 vowType9 : 1;
	u8 vowType10 : 1;
	u8 vowType11 : 1;
	u8 vowType12 : 1;
	u8 vowType13 : 1;
	u8 vowType14 : 1;
	u8 vowType15 : 1;
	u8 enable_live : 1;
	u8 enable_gray : 1;
	u8 enable_white : 1;
	u8 enable_black : 1;
	u8 enable_multi : 1;
	u8 disable_offline : 1;
	u8 isEquip : 1;
	u8 isConsume : 1;
	u8 isAutoEquip : 1;
	u8 isEstablishment : 1;
	u8 isOnlyOne : 1;
	u8 isDiscard : 1;
	u8 isDeposit : 1;
	u8 isDisableHand : 1;
	u8 isRemoveItem_forGameClear : 1;
	u8 isSuppleItem : 1;
	u8 isFullSuppleItem : 1;
	u8 isEnhance : 1;
	u8 isFixItem : 1;
	u8 disableMutliDropShare : 1;
	u8 disableUseAtColiseum : 1;
	u8 disableUseAtOutofColiseum : 1;
	u8 isEnableFastUseItem : 1;
	u8 isApplySpecialEffect : 1;
	u8 syncNumVaryId : 1;
	u8 isAshenEstusFlask : 1;
	u8 unknown1 : 1;
	u8 unknown2 : 1;
	u8 unknown3 : 1;
	u8 unknown4 : 1;
	u8 unknown5 : 1;
	u8 unknown6 : 1;
	s32 refId_1;
	s32 refVirtualWepId;
	s32 vagrantItemLotId;
	s32 vagrantBonusEneDropItemLotId;
	s32 vagrantItemEneDropItemLotId;
	s32 castSfxId;
	s32 fireSfxId;
	s32 effectSfxId;
	u8 enable_ActiveEmber : 1;
	u8 isBonfireWarpItem : 1;
	u8 enable_Ladder : 1;
	u8 isUseMultiPlayPreparation : 1;
	u8 canMultiUse : 1;
	u8 isShieldEnchant : 1;
	u8 isWarpProhibited : 1;
	u8 isUseMultiPenaltyOnly : 1;
	u8 suppleType;
	u8 autoReplenishType;
	u8 isDrop;
	s16 maxRepNum;
	u8 invadeType;
	dummy8 pad1[1];
	s32 shopId;
	s16 consumeMP;
	s16 useLimitCategory2;
	dummy8 pad2[8];
} EquipParamGoodsRow;

typedef struct _EquipParamProtectorRow {
	s32 sortId;
	s32 wanderingEquipId;
	s32 vagrantItemLotId;
	s32 vagrantBonuseneDropItemLotId;
	s32 vagrantItemEneDropItemLotId;
	s32 fixPrice;
	s32 basicPrice;
	s32 sellValue;
	f32 weight;
	s32 residentSpEffectId;
	s32 residentSpEffectId2;
	s32 residentSpEffectId3;
	s32 materialSetId;
	f32 partsDamageRate;
	f32 correctSARecover;
	s32 originEquipPro1;
	s32 originEquipPro2;
	s32 originEquipPro3;
	s32 originEquipPro4;
	s32 originEquipPro5;
	s32 originEquipPro6;
	s32 originEquipPro7;
	s32 originEquipPro8;
	s32 originEquipPro9;
	s32 originEquipPro10;
	s32 originEquipPro11;
	s32 originEquipPro12;
	s32 originEquipPro13;
	s32 originEquipPro14;
	s32 originEquipPro15;
	s32 originEquipPro16;
	f32 faceScaleM_ScaleX;
	f32 faceScaleM_ScaleZ;
	f32 faceScaleM_MaxX;
	f32 faceScaleM_MaxZ;
	f32 faceScaleF_ScaleX;
	f32 faceScaleF_ScaleZ;
	f32 faceScaleF_MaxX;
	f32 faceScaleF_MaxZ;
	s32 qwcId;
	s16 equipModelId;
	s16 iconIdM;
	s16 iconIdF;
	s16 Knockback;
	s16 KnockbackBounceRate;
	s16 durability;
	s16 durabilityMax;
	s16 saDurability;
	s16 defFlickPower;
	s16 defensePhys;
	s16 defenseMagic;
	s16 defenseFire;
	s16 defenseThunder;
	s16 defenseSlash;
	s16 defenseBlow;
	s16 defenseThrust;
	s16 resistPoison;
	s16 resistToxic;
	s16 resistBlood;
	s16 resistCurse;
	s16 reinforceTypeId;
	s16 compTrophySedId;
	s16 shopLv;
	u8 knockbackParamId;
	u8 FlickDamageCutRate;
	u8 equipModelCategory;
	u8 equipModelGender;
	u8 protectorCategory;
	u8 defenseMaterial;
	u8 defenseMaterialSfx;
	u8 partsDmgType;
	u8 defenseMaterial_Weak;
	u8 defenseMaterialSfx_Weak;
	u8 isDeposit : 1;
	u8 headEquip : 1;
	u8 bodyEquip : 1;
	u8 armEquip : 1;
	u8 legEquip : 1;
	u8 useFaceScale : 1;
	u8 InvisibleFlag00 : 1;
	u8 InvisibleFlag01 : 1;
	u8 InvisibleFlag02 : 1;
	u8 InvisibleFlag03 : 1;
	u8 InvisibleFlag04 : 1;
	u8 InvisibleFlag05 : 1;
	u8 InvisibleFlag06 : 1;
	u8 InvisibleFlag07 : 1;
	u8 InvisibleFlag08 : 1;
	u8 InvisibleFlag09 : 1;
	u8 InvisibleFlag10 : 1;
	u8 InvisibleFlag11 : 1;
	u8 InvisibleFlag12 : 1;
	u8 InvisibleFlag13 : 1;
	u8 InvisibleFlag14 : 1;
	u8 InvisibleFlag15 : 1;
	u8 InvisibleFlag16 : 1;
	u8 InvisibleFlag17 : 1;
	u8 InvisibleFlag18 : 1;
	u8 InvisibleFlag19 : 1;
	u8 InvisibleFlag20 : 1;
	u8 InvisibleFlag21 : 1;
	u8 InvisibleFlag22 : 1;
	u8 InvisibleFlag23 : 1;
	u8 InvisibleFlag24 : 1;
	u8 InvisibleFlag25 : 1;
	u8 InvisibleFlag26 : 1;
	u8 InvisibleFlag27 : 1;
	u8 InvisibleFlag28 : 1;
	u8 InvisibleFlag29 : 1;
	u8 InvisibleFlag30 : 1;
	u8 InvisibleFlag31 : 1;
	u8 InvisibleFlag32 : 1;
	u8 InvisibleFlag33 : 1;
	u8 InvisibleFlag34 : 1;
	u8 InvisibleFlag35 : 1;
	u8 InvisibleFlag36 : 1;
	u8 InvisibleFlag37 : 1;
	u8 InvisibleFlag38 : 1;
	u8 InvisibleFlag39 : 1;
	u8 InvisibleFlag40 : 1;
	u8 InvisibleFlag41 : 1;
	u8 InvisibleFlag42 : 1;
	u8 InvisibleFlag43 : 1;
	u8 InvisibleFlag44 : 1;
	u8 InvisibleFlag45 : 1;
	u8 InvisibleFlag46 : 1;
	u8 InvisibleFlag47 : 1;
	u8 disableMultiDropShare : 1;
	u8 simpleModelForDlc1 : 1;
	u8 isGuestDrop0 : 1;
	u8 isGuestDrop1 : 1;
	u8 showLogCondType : 1;
	u8 showDialogCondType : 1;
	u8 IsDiscard : 1;
	u8 IsDrop : 1;
	u8 Unk4 : 1;
	u8 Unk5 : 1;
	f32 PhysDamageCutRate;
	f32 SlashDamageCutRate;
	f32 StrikeDamageCutRate;
	f32 ThrustDamageCutRate;
	f32 MagicDamageCutRate;
	f32 FireDamageCutRate;
	f32 ThunderDamageCutRate;
	u16 defenseMaterialSfx1;
	u16 defenseMaterialSfx_Weak1;
	u16 defenseMaterial1;
	u16 defenseMaterial_Weak1;
	u16 defenseMaterialSfx2;
	u16 defenseMaterialSfx_Weak2;
	u16 footMaterialSe;
	u16 defenseMaterial_Weak2;
	s32 autoFootEffectDecalBaseId1;
	f32 toughnessDamageCutRate;
	dummy8 pad1[4];
	f32 DarkDamageCutRate;
	s16 defenseDark;
	u8 Unk6;
	u8 Unk7;
	s32 Unk8;
	s32 postureControlId;
	s32 saleValue;
	s16 resistFrost;
	u8 Mask00;
	u8 Mask01;
	u8 Mask02;
	u8 Mask03;
	u8 Mask04;
	u8 Mask05;
	u8 Mask06;
	u8 Mask07;
	u8 Mask08;
	u8 Mask09;
	u8 Mask10;
	u8 Mask11;
	u8 Mask12;
	u8 Mask13;
	u8 Mask14;
	u8 Mask15;
	u8 Mask16;
	u8 Mask17;
	u8 Mask18;
	u8 Mask19;
	u8 Mask20;
	u8 Mask21;
	u8 Mask22;
	u8 Mask23;
	u8 Mask24;
	u8 Mask25;
	u8 Mask26;
	u8 Mask27;
	u8 Mask28;
	u8 Mask29;
	u8 Mask30;
	u8 Mask31;
	u8 Mask32;
	u8 Mask33;
	u8 Mask34;
	u8 Mask35;
	u8 Mask36;
	u8 Mask37;
	u8 Mask38;
	u8 Mask39;
	u8 Mask40;
	u8 Mask41;
	u8 Mask42;
	u8 Mask43;
	u8 Mask44;
	u8 Mask45;
	u8 Mask46;
	u8 Mask47;
	u8 Mask48;
	u8 Mask49;
	u8 Mask50;
	u8 Mask51;
	u8 Mask52;
	u8 Mask53;
	u8 Mask54;
	u8 Mask55;
	u8 Mask56;
	u8 Mask57;
	u8 Mask58;
	u8 Mask59;
	u8 Mask60;
	u8 Mask61;
	u8 Mask62;
	u8 Mask63;
	u8 Mask64;
	u8 Mask65;
	u8 Mask66;
	u8 Mask67;
	u8 Mask68;
	u8 Mask69;
	u8 Mask70;
	u8 Mask71;
	u8 Mask72;
	u8 Mask73;
	u8 Mask74;
	u8 Mask75;
	u8 Mask76;
	u8 Mask77;
	u8 Mask78;
	u8 Mask79;
	u8 Mask80;
	u8 Mask81;
	u8 Mask82;
	u8 Mask83;
	u8 Mask84;
	u8 Mask85;
	u8 Mask86;
	u8 Mask87;
	u8 Mask88;
	u8 Mask89;
	u8 Mask90;
	u8 Mask91;
	u8 Mask92;
	u8 Mask93;
	u8 Mask94;
	u8 Mask95;
	u8 Mask96;
	u8 Mask97;
} EquipParamProtectorRow;

typedef struct _EquipParamWeaponRow {
	s32 behaviorVariationId;
	s32 sortId;
	s32 wanderingEquipId;
	f32 weight;
	f32 weaponWeightRate;
	s32 fixPrice;
	s32 basicPrice;
	s32 sellValue;
	f32 correctStrength;
	f32 correctAgility;
	f32 corretMagic;
	f32 corretFaith;
	f32 physGuardCutRate;
	f32 magGuardCutRate;
	f32 fireGuardCutRate;
	f32 thunGuardCutRate;
	s32 spEffectBehaviorId0;
	s32 spEffectBehaviorId1;
	s32 spEffectBehaviorId2;
	s32 residentSpEffectId0;
	s32 residentSpEffectId1;
	s32 residentSpEffectId2;
	s32 materialSetId;
	s32 originEquipWep0;
	s32 originEquipWep1;
	s32 originEquipWep2;
	s32 originEquipWep3;
	s32 originEquipWep4;
	s32 originEquipWep5;
	s32 originEquipWep6;
	s32 originEquipWep7;
	s32 originEquipWep8;
	s32 originEquipWep9;
	s32 originEquipWep10;
	s32 originEquipWep11;
	s32 originEquipWep12;
	s32 originEquipWep13;
	s32 originEquipWep14;
	s32 originEquipWep15;
	f32 weakA_DamageRate;
	f32 weakB_DamageRate;
	f32 weakC_DamageRate;
	f32 weakD_DamageRate;
	s32 vagrantItemLotId;
	s32 vagrantBonuseneDropItemLotId;
	s32 vagrantItemEneDropItemLotId;
	s16 equipModelId;
	s16 iconId;
	s16 durability;
	s16 durailityMax;
	s16 attackThrowEscape;
	s16 parryDamageLife;
	s16 atkBasePhysics;
	s16 atkBaseMagic;
	s16 atkBaseFire;
	s16 atkBaseThunder;
	s16 atkBaseStamina;
	s16 saWeaponDamage;
	s16 saDurability;
	s16 guardAngle;
	s16 staminaGuardDef;
	s16 reinforceTypeId;
	s16 trophySGradeId;
	s16 trophySeqId;
	s16 throwAtkRate;
	s16 bowDistRate;
	u8 equipModelCategory;
	u8 equipModelGender;
	u8 weaponCategory;
	u8 wepmotionCategory;
	u8 guardmotionCategory;
	u8 atkMaterial;
	u8 defMaterial;
	u8 defSfxMaterial;
	u8 correctType;
	u8 spAttribute;
	s16 spAtkCategory;
	u8 wepmotionOneHandId;
	u8 wepmotionBothHandId;
	u8 properStrength;
	u8 properAgility;
	u8 properMagic;
	u8 properFaith;
	u8 overStrength;
	u8 attackBaseParry;
	u8 defenseBaseParry;
	u8 guardBaseRepel;
	u8 attackBaseRepel;
	u8 guardCutCancelRate;
	u8 guardLevel;
	u8 slashGuardCutRate;
	u8 blowGuardCutRate;
	u8 thrustGuardCutRate;
	u8 poisonGuardResist;
	u8 toxicGuardResist;
	u8 bloodGuardResist;
	u8 curseGuardResist;
	u8 atkAttribute;
	u8 rightHandEquipable : 1;
	u8 leftHandEquipable : 1;
	u8 bothHandEquipable : 1;
	u8 arrowSlotEquipable : 1;
	u8 boltSlotEquipable : 1;
	u8 enableGuard : 1;
	u8 enableParry : 1;
	u8 enableMagic : 1;
	u8 enablePyromancy : 1;
	u8 enableMiracle : 1;
	u8 enableVowMagic : 1;
	u8 isNormalAttackType : 1;
	u8 isBlowAttackType : 1;
	u8 isSlashAttackType : 1;
	u8 isThrustAttackType : 1;
	u8 isEnhance : 1;
	u8 isHeroPointCorrect : 1;
	u8 isCustom : 1;
	u8 disableBaseChangeReset : 1;
	u8 disableRepair : 1;
	u8 isDarkHand : 1;
	u8 simpleModelForDlc : 1;
	u8 lanternWep : 1;
	u8 isVersusGhostWep : 1;
	u8 infusion_icon_a : 1;
	u8 infusion_icon_b : 1;
	u8 infusion_icon_c : 1;
	u8 infusion_icon_d : 1;
	u8 infusion_icon_e : 1;
	u8 infusion_icon_f : 1;
	u8 DisableShoot : 1;
	u8 EnableRepository : 1;
	u8 DisableMultiDropShare2 : 1;
	u8 IsDiscard : 1;
	u8 IsDrop : 1;
	u8 showLogCondType : 1;
	u8 enableThrow : 1;
	u8 showDialogCondType : 2;
	u8 disableGemAttr : 1;
	u16 defSfxMaterial1;
	u8 wepCollidableType0;
	u8 wepCollidableType1;
	u8 postureControlId_Right;
	u8 postureControlId_Left;
	s32 Group0_AtkVfxId;
	s32 Group0_DummyPolyId0;
	s32 Group0_DummyPolyId1;
	s32 Group1_AtkVfxId;
	s32 Group1_DummyPolyId0;
	s32 Group1_DummyPolyId1;
	s32 Group2_AtkVfxId;
	s32 Group2_DummyPolyId0;
	s32 Group2_DummyPolyId1;
	s32 Group3_AtkVfxId;
	s32 Group3_DummyPolyId0;
	s32 Group3_DummyPolyId1;
	s32 Group4_AtkVfxId;
	s32 Group4_DummyPolyId0;
	s32 Group4_DummyPolyId1;
	s32 Group5_AtkVfxId;
	s32 Group5_DummyPolyId0;
	s32 Group5_DummyPolyId1;
	s32 Group6_AtkVfxId;
	s32 Group6_DummyPolyId0;
	s32 Group6_DummyPolyId1;
	s32 Group7_AtkVfxId;
	s32 Group7_DummyPolyId0;
	s32 Group7_DummyPolyId1;
	s16 defSfxMaterial2;
	s16 defSeMaterial2;
	s32 absorpParamId;
	f32 toughnessCorrectRate;
	u8 isValidTough_ProtSADmg : 1;
	u8 isDualBlade : 1;
	u8 isAutoEquip : 1;
	u8 isEnableEmergencyStep : 1;
	u8 invisibleOnRemo : 1;
	dummy8 pad2 : 3;
	u8 correctType_Magic;
	u8 correctType_Fire;
	u8 correctType_Thunder;
	f32 weakE_DamageRate;
	f32 weakF_DamageRate;
	f32 darkGuardCutRate;
	s16 atkBaseDark;
	u8 correctType_Dark;
	u8 correctType_Poison;
	s32 swordArt_ActId;
	u8 correctType_Bleed;
	u8 properLuck;
	u8 freezeGuardResist;
	u8 autoReplenishType;
	s32 swordArtsParamId;
	f32 correctLuck;
	u32 arrowBoltEquipId;
	u8 DerivationLevelType;
	u8 enchantSfxSize;
	s16 displayTypeId;
	f32 physGuardCutRate_MaxCorrect;
	f32 magGuardCutRate_MaxCorrect;
	f32 fireGuardCutRate_MaxCorrect;
	f32 thunGuardCutRate_MaxCorrect;
	f32 darkGuardCutRate_MaxCorrect;
	f32 poisonGuardResist_MaxCorrect;
	f32 diseaseGuardResist_MaxCorrect;
	f32 bloodGuardResist_MaxCorrect;
	f32 curseGuardResist_MaxCorrect;
	f32 freezeGuardResist_MaxCorrect;
	f32 staminaGuardDef_MaxCorrect;
	s32 weaponVfx0;
	s32 weaponVfx1;
	s32 weaponVfx2;
	s32 weaponVfx3;
	s32 weaponVfx4;
	s32 weaponVfx5;
	s32 weaponVfx6;
	s32 weaponVfx7;
	f32 StaminaConsumeRate;
	f32 vsPlayerDmgCorrectRate_Physics;
	f32 vsPlayerDmgCorrectRate_Magic;
	f32 vsPlayerDmgCorrectRate_Fire;
	f32 vsPlayerDmgCorrectRate_Thunder;
	f32 vsPlayerDmgCorrectRate_Dark;
	f32 vsPlayerDmgCorrectRate_Poison;
	f32 vsPlayerDmgCorrectRate_Blood;
	f32 vsPlayerDmgCorrectRate_Freeze;
	s32 attainmentWepStatusStr;
	s32 attainmentWepStatusDex;
	s32 attainmentWepStatusMag;
	s32 attainmentWepStatusFai;
	s32 attainmentWepStatusLuc;
	s32 attackElementCorrectId;
	s32 saleValue;
	u8 reinforceShopCategory;
	u8 maxArrowQuantity;
	u8 residentSfx_1_IsVisibleForHang : 1;
	u8 residentSfx_2_IsVisibleForHang : 1;
	u8 residentSfx_3_IsVisibleForHang : 1;
	u8 residentSfx_4_IsVisibleForHang : 1;
	u8 isSoulParamIdChange_model0 : 1;
	u8 isSoulParamIdChange_model1 : 1;
	u8 isSoulParamIdChange_model2 : 1;
	u8 isSoulParamIdChange_model3 : 1;
	u8 wepSeIdOffset;
	s32 baseChangePrice;
	s16 levelSyncCorrectId;
	dummy8 pad1[38];
} EquipParamWeaponRow;

struct CSRegulationManager : public FD4Singleton<CSRegulationManager, "CSRegulationManager"> {
	void** vftable_ptr;
	uint8_t pad00[0x08];
	ParamResCap** param_list_begin;
	ParamResCap** param_list_end;
};

// Returns the address of the parameter data in the table with the given ID, or NULL if there is no
// such row.
inline LPVOID* FindRow(ParamTable* table, uint64_t rowId) {
	uint16_t rows = table->num_rows;
	for (uint16_t i = 0; i < rows; i++) {
		ParamRowInfo* row = &table->rows[i];
		if (row->row_id == rowId) {
			return (LPVOID*)((char*)table + row->param_offset);
		}
	}
	return NULL;
}

// Returns the param data for the ring with the given ID (which should he the base ID without
// the category flag). Prints an error and returns NULL if this can't be loaded.
inline EquipParamAccessoryRow* GetAccessoryParam(DWORD id) {
	CSRegulationManager* manager = CSRegulationManager::instance();
	ParamTable* equipParamAccessory = manager->param_list_begin[23]->param_header->param_table;
	EquipParamAccessoryRow* row = (EquipParamAccessoryRow*)FindRow(equipParamAccessory, id);
	if (id == NULL) {
		printf("ERROR: Failed to locate item ID %d, randomizer may not be set up correctly\n", id);
		return NULL;
	}
	else {
		return row;
	}
}

// Returns the param data for the goods item with the given ID (which should he the base ID without
// the category flag). Prints an error and returns NULL if this can't be loaded.
inline EquipParamGoodsRow* GetGoodsParam(DWORD id) {
	CSRegulationManager* manager = CSRegulationManager::instance();
	ParamTable* equipParamGoods = manager->param_list_begin[24]->param_header->param_table;
	EquipParamGoodsRow* row = (EquipParamGoodsRow*)FindRow(equipParamGoods, id);
	if (id == NULL) {
		printf("ERROR: Failed to locate item ID %d, randomizer may not be set up correctly\n", id);
		return NULL;
	}
	else {
		return row;
	}
}

// Returns the param data for the armor with the given ID (which should he the base ID without
// the category flag). Prints an error and returns NULL if this can't be loaded.
inline EquipParamProtectorRow* GetProtectorParam(DWORD id) {
	CSRegulationManager* manager = CSRegulationManager::instance();
	ParamTable* equipParamProtector = manager->param_list_begin[25]->param_header->param_table;
	EquipParamProtectorRow* row = (EquipParamProtectorRow*)FindRow(equipParamProtector, id);
	if (id == NULL) {
		printf("ERROR: Failed to locate item ID %d, randomizer may not be set up correctly\n", id);
		return NULL;
	}
	else {
		return row;
	}
}

// Returns the param data for the weapon with the given ID (which should he the base ID without
// the category flag). Prints an error and returns NULL if this can't be loaded.
inline EquipParamWeaponRow* GetWeaponParam(DWORD id) {
	CSRegulationManager* manager = CSRegulationManager::instance();
	ParamTable* equipParamWeapon = manager->param_list_begin[26]->param_header->param_table;
	EquipParamWeaponRow* row = (EquipParamWeaponRow*)FindRow(equipParamWeapon, id - id % 100);
	if (id == NULL) {
		printf("ERROR: Failed to locate item ID %d, randomizer may not be set up correctly\n", id);
		return NULL;
	}
	else {
		return row;
	}
}