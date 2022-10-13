#pragma once

#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }

enum ECompressionFlags
{
	/** No compression																*/
	COMPRESS_None = 0x00,
	/** Compress with ZLIB - DEPRECATED, USE FNAME									*/
	COMPRESS_ZLIB = 0x01,
	/** Compress with GZIP - DEPRECATED, USE FNAME									*/
	COMPRESS_GZIP = 0x02,
	/** Compress with user defined callbacks - DEPRECATED, USE FNAME                */
	COMPRESS_Custom = 0x04,
	/** Joint of the previous ones to determine if old flags are being used			*/
	COMPRESS_DeprecatedFormatFlagsMask = 0xF,


	/** No flags specified /														*/
	COMPRESS_NoFlags = 0x00,
	/** Prefer compression that compresses smaller (ONLY VALID FOR COMPRESSION)		*/
	COMPRESS_BiasMemory = 0x10,
	COMPRESS_BiasSize = COMPRESS_BiasMemory,
	/** Prefer compression that compresses faster (ONLY VALID FOR COMPRESSION)		*/
	COMPRESS_BiasSpeed = 0x20,
	/** Is the source buffer padded out	(ONLY VALID FOR UNCOMPRESS)					*/
	COMPRESS_SourceIsPadded = 0x80,

	/** Set of flags that are options are still allowed								*/
	COMPRESS_OptionsFlagsMask = 0xF0,

	/** Indicate this compress call is for Packaging (pak/iostore) */
	COMPRESS_ForPackaging = 0x100,
	COMPRESS_ForPurposeMask = 0xF00,
};

enum class EIoStoreTocVersion : uint8_t
{
	Invalid = 0,
	Initial,
	DirectoryIndex,
	PartitionSize,
	PerfectHash,
	PerfectHashWithOverflow,
	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

enum class EIoStoreTocReadOptions
{
	Default,
	ReadDirectoryIndex = (1 << 0),
	ReadTocMeta = (1 << 1),
	ReadAll = ReadDirectoryIndex | ReadTocMeta
};

enum class FIoStoreTocEntryMetaFlags : uint8_t
{
	None,
	Compressed = (1 << 0),
	MemoryMapped = (1 << 1)
};

enum class EIoContainerFlags : uint8_t
{
	None,
	Compressed = (1 << 0),
	Encrypted = (1 << 1),
	Signed = (1 << 2),
	Indexed = (1 << 3),
};
ENUM_CLASS_FLAGS(EIoContainerFlags);

enum ELifetimeCondition
{
	COND_None = 0,		// This property has no condition, and will send anytime it changes
	COND_InitialOnly = 1,	// This property will only attempt to send on the initial bunch
	COND_OwnerOnly = 2,		// This property will only send to the actor's owner
	COND_SkipOwner = 3,	// This property send to every connection EXCEPT the owner
	COND_SimulatedOnly = 4,	// This property will only send to simulated actors
	COND_AutonomousOnly = 5,  // This property will only send to autonomous actors
	COND_SimulatedOrPhysics = 6,  // This property will send to simulated OR bRepPhysics actors
	COND_InitialOrOwner = 7,	// This property will send on the initial packet, or to the actors owner
	COND_Custom = 8,	// This property has no particular condition, but wants the ability to toggle on/off via SetCustomIsActiveOverride
	COND_ReplayOrOwner = 9,	// This property will only send to the replay connection, or to the actors owner
	COND_ReplayOnly = 10,	// This property will only send to the replay connection
	COND_SimulatedOnlyNoReplay = 11,	// This property will send to actors only, but not to replay connections
	COND_SimulatedOrPhysicsNoReplay = 12,  // This property will send to simulated Or bRepPhysics actors, but not to replay connections
	COND_SkipReplay = 13,  // This property will not send to the replay connection
	COND_Never = 15, // This property will never be replicated
	COND_Max = 16
};

enum EPropertyFlags : uint64_t
{
	CPF_None = 0,

	CPF_Edit = 0x0000000000000001,	///< Property is user-settable in the editor.
	CPF_ConstParm = 0x0000000000000002,	///< This is a constant function parameter
	CPF_BlueprintVisible = 0x0000000000000004,	///< This property can be read by blueprint code
	CPF_ExportObject = 0x0000000000000008,	///< Object can be exported with actor.
	CPF_BlueprintReadOnly = 0x0000000000000010,	///< This property cannot be modified by blueprint code
	CPF_Net = 0x0000000000000020,	///< Property is relevant to network replication.
	CPF_EditFixedSize = 0x0000000000000040,	///< Indicates that elements of an array can be modified, but its size cannot be changed.
	CPF_Parm = 0x0000000000000080,	///< Function/When call parameter.
	CPF_OutParm = 0x0000000000000100,	///< Value is copied out after function call.
	CPF_ZeroConstructor = 0x0000000000000200,	///< memset is fine for construction
	CPF_ReturnParm = 0x0000000000000400,	///< Return value.
	CPF_DisableEditOnTemplate = 0x0000000000000800,	///< Disable editing of this property on an archetype/sub-blueprint
	//CPF_      						= 0x0000000000001000,	///< 
	CPF_Transient = 0x0000000000002000,	///< Property is transient: shouldn't be saved or loaded, except for Blueprint CDOs.
	CPF_Config = 0x0000000000004000,	///< Property should be loaded/saved as permanent profile.
	//CPF_								= 0x0000000000008000,	///< 
	CPF_DisableEditOnInstance = 0x0000000000010000,	///< Disable editing on an instance of this class
	CPF_EditConst = 0x0000000000020000,	///< Property is uneditable in the editor.
	CPF_GlobalConfig = 0x0000000000040000,	///< Load config from base class, not subclass.
	CPF_InstancedReference = 0x0000000000080000,	///< Property is a component references.
	//CPF_								= 0x0000000000100000,	///<
	CPF_DuplicateTransient = 0x0000000000200000,	///< Property should always be reset to the default value during any type of duplication (copy/paste, binary duplication, etc.)
	//CPF_								= 0x0000000000400000,	///< 
	//CPF_    							= 0x0000000000800000,	///< 
	CPF_SaveGame = 0x0000000001000000,	///< Property should be serialized for save games, this is only checked for game-specific archives with ArIsSaveGame
	CPF_NoClear = 0x0000000002000000,	///< Hide clear (and browse) button.
	//CPF_  							= 0x0000000004000000,	///<
	CPF_ReferenceParm = 0x0000000008000000,	///< Value is passed by reference; CPF_OutParam and CPF_Param should also be set.
	CPF_BlueprintAssignable = 0x0000000010000000,	///< MC Delegates only.  Property should be exposed for assigning in blueprint code
	CPF_Deprecated = 0x0000000020000000,	///< Property is deprecated.  Read it from an archive, but don't save it.
	CPF_IsPlainOldData = 0x0000000040000000,	///< If this is set, then the property can be memcopied instead of CopyCompleteValue / CopySingleValue
	CPF_RepSkip = 0x0000000080000000,	///< Not replicated. For non replicated properties in replicated structs 
	CPF_RepNotify = 0x0000000100000000,	///< Notify actors when a property is replicated
	CPF_Interp = 0x0000000200000000,	///< interpolatable property for use with cinematics
	CPF_NonTransactional = 0x0000000400000000,	///< Property isn't transacted
	CPF_EditorOnly = 0x0000000800000000,	///< Property should only be loaded in the editor
	CPF_NoDestructor = 0x0000001000000000,	///< No destructor
	//CPF_								= 0x0000002000000000,	///<
	CPF_AutoWeak = 0x0000004000000000,	///< Only used for weak pointers, means the export type is autoweak
	CPF_ContainsInstancedReference = 0x0000008000000000,	///< Property contains component references.
	CPF_AssetRegistrySearchable = 0x0000010000000000,	///< asset instances will add properties with this flag to the asset registry automatically
	CPF_SimpleDisplay = 0x0000020000000000,	///< The property is visible by default in the editor details view
	CPF_AdvancedDisplay = 0x0000040000000000,	///< The property is advanced and not visible by default in the editor details view
	CPF_Protected = 0x0000080000000000,	///< property is protected from the perspective of script
	CPF_BlueprintCallable = 0x0000100000000000,	///< MC Delegates only.  Property should be exposed for calling in blueprint code
	CPF_BlueprintAuthorityOnly = 0x0000200000000000,	///< MC Delegates only.  This delegate accepts (only in blueprint) only events with BlueprintAuthorityOnly.
	CPF_TextExportTransient = 0x0000400000000000,	///< Property shouldn't be exported to text format (e.g. copy/paste)
	CPF_NonPIEDuplicateTransient = 0x0000800000000000,	///< Property should only be copied in PIE
	CPF_ExposeOnSpawn = 0x0001000000000000,	///< Property is exposed on spawn
	CPF_PersistentInstance = 0x0002000000000000,	///< A object referenced by the property is duplicated like a component. (Each actor should have an own instance.)
	CPF_UObjectWrapper = 0x0004000000000000,	///< Property was parsed as a wrapper class like TSubclassOf<T>, FScriptInterface etc., rather than a USomething*
	CPF_HasGetValueTypeHash = 0x0008000000000000,	///< This property can generate a meaningful hash value.
	CPF_NativeAccessSpecifierPublic = 0x0010000000000000,	///< Public native access specifier
	CPF_NativeAccessSpecifierProtected = 0x0020000000000000,	///< Protected native access specifier
	CPF_NativeAccessSpecifierPrivate = 0x0040000000000000,	///< Private native access specifier
	CPF_SkipSerialization = 0x0080000000000000,	///< Property shouldn't be serialized, can still be exported to text
};

enum EObjectFlags
{
	RF_NoFlags = 0x0,
	RF_Public = 0x1,
	RF_Standalone = 0x2,
	RF_MarkAsNative = 0x4,
	RF_Transactional = 0x8,
	RF_ClassDefaultObject = 0x10,
	RF_ArchetypeObject = 0x20,
	RF_Transient = 0x40,
	RF_MarkAsRootSet = 0x80,
	RF_TagGarbageTemp = 0x100,
	RF_NeedInitialization = 0x200,
	RF_NeedLoad = 0x400,
	RF_KeepForCooker = 0x800,
	RF_NeedPostLoad = 0x1000,
	RF_NeedPostLoadSubobjects = 0x2000,
	RF_NewerVersionExists = 0x4000,
	RF_BeginDestroyed = 0x8000,
	RF_FinishDestroyed = 0x10000,
	RF_BeingRegenerated = 0x20000,
	RF_DefaultSubObject = 0x40000,
	RF_WasLoaded = 0x80000,
	RF_TextExportTransient = 0x100000,
	RF_LoadCompleted = 0x200000,
	RF_InheritableComponentTemplate = 0x400000,
	RF_DuplicateTransient = 0x800000,
	RF_StrongRefOnFrame = 0x1000000,
	RF_NonPIEDuplicateTransient = 0x2000000,
	RF_Dynamic = 0x4000000,
	RF_WillBeLoaded = 0x8000000,
	RF_HasExternalPackage = 0x10000000,
};

enum class FFieldClassID : uint64_t
{
	Int8 = 1llu << 1,
	Byte = 1llu << 6,
	Int = 1llu << 7,
	Float = 1llu << 8,
	UInt64 = 1llu << 9,
	Class = 1llu << 10,
	UInt32 = 1llu << 11,
	Interface = 1llu << 12,
	Name = 1llu << 13,
	String = 1llu << 14,
	Object = 1llu << 16,
	Bool = 1llu << 17,
	UInt16 = 1llu << 18,
	Struct = 1llu << 20,
	Array = 1llu << 21,
	Int64 = 1llu << 22,
	Delegate = 1llu << 23,
	SoftObject = 1llu << 27,
	LazyObject = 1llu << 28,
	WeakObject = 1llu << 29,
	Text = 1llu << 30,
	Int16 = 1llu << 31,
	Double = 1llu << 32,
	SoftClass = 1llu << 33,
	Map = 1llu << 46,
	Set = 1llu << 47,
	Enum = 1llu << 48,
	MulticastInlineDelegate = 1llu << 50,
	MulticastSparseDelegate = 1llu << 51,
	ObjectPointer = 1llu << 53
};

enum EClassFlags
{
	CLASS_None = 0x00000000u,
	CLASS_Abstract = 0x00000001u,
	CLASS_DefaultConfig = 0x00000002u,
	CLASS_Config = 0x00000004u,
	CLASS_Transient = 0x00000008u,
	CLASS_Parsed = 0x00000010u,
	CLASS_MatchedSerializers = 0x00000020u,
	CLASS_AdvancedDisplay = 0x00000040u,
	CLASS_Native = 0x00000080u,
	CLASS_NoExport = 0x00000100u,
	CLASS_NotPlaceable = 0x00000200u,
	CLASS_PerObjectConfig = 0x00000400u,
	CLASS_ReplicationDataIsSetUp = 0x00000800u,
	CLASS_EditInlineNew = 0x00001000u,
	CLASS_CollapseCategories = 0x00002000u,
	CLASS_Interface = 0x00004000u,
	CLASS_CustomConstructor = 0x00008000u,
	CLASS_Const = 0x00010000u,
	CLASS_LayoutChanging = 0x00020000u,
	CLASS_CompiledFromBlueprint = 0x00040000u,
	CLASS_MinimalAPI = 0x00080000u,
	CLASS_RequiredAPI = 0x00100000u,
	CLASS_DefaultToInstanced = 0x00200000u,
	CLASS_TokenStreamAssembled = 0x00400000u,
	CLASS_HasInstancedReference = 0x00800000u,
	CLASS_Hidden = 0x01000000u,
	CLASS_Deprecated = 0x02000000u,
	CLASS_HideDropDown = 0x04000000u,
	CLASS_GlobalUserConfig = 0x08000000u,
	CLASS_Intrinsic = 0x10000000u,
	CLASS_Constructed = 0x20000000u,
	CLASS_ConfigDoNotCheckDefaults = 0x40000000u,
	CLASS_NewerVersionExists = 0x80000000u,
};

enum class EIoContainerHeaderVersion : uint32_t
{
	Initial = 0,
	LocalizedPackages = 1,
	OptionalSegmentPackages = 2,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};