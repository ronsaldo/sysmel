Class {
	#name : #MbndSlovimCompilationTarget,
	#superclass : #MbndAbstractConcreteCompilationTarget,
	#instVars : [
		'slvmCompilationTarget',
		'targetName'
	],
	#category : #'SysmelMoebiusCodeGenerator-Slovim-CompilationTarget'
}

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> forWBTPlatform: wbtPlatform [
	^ self basicNew
		initializeWithWBTPlatform: wbtPlatform;
		yourself
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_arm [
	^ self slvmCompilationTargetNamed: #llvm_arm
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_arm64 [
	^ self slvmCompilationTargetNamed: #llvm_arm64
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv5 [
	^ self slvmCompilationTargetNamed: #llvm_armv5
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv6 [
	^ self slvmCompilationTargetNamed: #llvm_armv6
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv6k [
	^ self slvmCompilationTargetNamed: #llvm_armv6k
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv6m [
	^ self slvmCompilationTargetNamed: #llvm_armv6m
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv7 [
	^ self slvmCompilationTargetNamed: #llvm_armv7
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv7a [
	^ self slvmCompilationTargetNamed: #llvm_armv7a
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_armv7m [
	^ self slvmCompilationTargetNamed: #llvm_armv7m
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_mipsel [
	^ self slvmCompilationTargetNamed: #llvm_mipsel
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_webAssembly [
	^ self slvmCompilationTargetNamed: #llvm_webAssembly
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_webAssembly32 [
	^ self slvmCompilationTargetNamed: #llvm_webAssembly32
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_webAssembly64 [
	^ self slvmCompilationTargetNamed: #llvm_webAssembly64
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_x86 [
	^ self slvmCompilationTargetNamed: #llvm_x86
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> llvm_x86_64 [
	^ self slvmCompilationTargetNamed: #llvm_x86_64
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> sasm_x86 [
	^ self slvmCompilationTargetNamed: #x86
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> sasm_x86_64 [
	^ self slvmCompilationTargetNamed: #x86_64
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> slvmCompilationTargetNamed: slvmTargetName [
	^ self basicNew
		slvmCompilationTargetNamed: slvmTargetName;
		yourself
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget class >> spirv_vulkan [
	^ self slvmCompilationTargetNamed: #spirv_logical64
]

{ #category : #adding }
MbndSlovimCompilationTarget >> addSysmelLLVMGCPlugin [
	| path |
	#(
		'SysmelGCPlugin.so'
		'SysmelGCPlugin.dylib'
		'SysmelGCPlugin.dll'
	) do: [ :pluginName |
		path := Smalltalk imageDirectory / pluginName.
		path exists ifTrue: [
			^ slvmCompilationTarget compilationPlugins: { path fullName }
		]
	]

]

{ #category : #converting }
MbndSlovimCompilationTarget >> asCleanCompilationTarget [
	| cleanTarget |
	cleanTarget := self class slvmCompilationTargetNamed: targetName.
	cleanTarget
		machineAttributes: self machineAttributes;
		operatingSystemName: self operatingSystemName;
		canonicalOperatingSystemName: self canonicalOperatingSystemName;
		systemVendor: self systemVendor;
		exceptionModel: self exceptionModel;
		cpu: self cpu;
		fpu: self fpu;
		floatAbiName: self floatAbiName;
		floatingPointModel: self floatingPointModel.
	cleanTarget slvmCompilationTarget
		emittingDebugInformation: slvmCompilationTarget emittingDebugInformation.
		
	^ cleanTarget
]

{ #category : #converting }
MbndSlovimCompilationTarget >> asMultiSpirVCompilationTarget [
	| result |
	result := self copy slvmCompilationTargetNamed: (self pointerSize = 4
		ifTrue: [#multiSpirv_logical32]
		ifFalse: [#multiSpirv_logical64]).
	self isEmittingDebugInformation ifTrue: [ result withDebugInformation ].
	^ result
]

{ #category : #converting }
MbndSlovimCompilationTarget >> asSpirVCompilationTarget [
	| result |
	result := self copy slvmCompilationTargetNamed: (self pointerSize = 4
		ifTrue: [#spirv_logical32]
		ifFalse: [#spirv_logical64]).
	self isEmittingDebugInformation ifTrue: [ result withDebugInformation ].
	^ result
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> canonicalOperatingSystemName [
	^ slvmCompilationTarget canonicalOperatingSystemName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> canonicalOperatingSystemName: osName [
	slvmCompilationTarget canonicalOperatingSystemName: osName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> cpu [
	^ slvmCompilationTarget cpu
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> cpu: targetCPU [
	slvmCompilationTarget cpu: targetCPU
]

{ #category : #testing }
MbndSlovimCompilationTarget >> exceptionModel [
	^ slvmCompilationTarget exceptionModel
]

{ #category : #testing }
MbndSlovimCompilationTarget >> exceptionModel: anExceptionModelName [
	slvmCompilationTarget exceptionModel: anExceptionModelName
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget >> extensionForModuleOutputMode: mode [
	slvmCompilationTarget isSpirV ifTrue: [
		mode = #executable ifTrue: [ ^ '.spv' ]
	].

	slvmCompilationTarget isWebAssembly ifTrue: [
		mode = #shared ifTrue: [ ^ '.wasm' ].
		mode = #executable ifTrue: [ ^ '.html' ].
	].

	^ super extensionForModuleOutputMode: mode
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> float64Alignment [
	^ slvmCompilationTarget float64 alignment
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> floatAbiName [
	^ slvmCompilationTarget floatAbiName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> floatAbiName: targetFloatAbiName [
	slvmCompilationTarget floatAbiName: targetFloatAbiName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> floatingPointModel [
	^ slvmCompilationTarget floatingPointModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> floatingPointModel: targetFloatAbiName [
	slvmCompilationTarget floatingPointModel: targetFloatAbiName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> fpu [
	^ slvmCompilationTarget fpu
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> fpu: targetFPU [
	slvmCompilationTarget fpu: targetFPU
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasArmEhAbi [
	^ slvmCompilationTarget hasArmEhAbi
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasDwarfExceptions [
	^ slvmCompilationTarget hasDwarfExceptions
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasNoExceptionModel [
	^ slvmCompilationTarget hasNoExceptionModel
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> hasNoOperatingSystem [
	^ self wbtPlatform isNoOperatingSystem
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasSEHExceptions [
	^ slvmCompilationTarget hasSEHExceptions
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasWebAssemblyExceptions [
	^ slvmCompilationTarget hasWebAssemblyExceptions
]

{ #category : #testing }
MbndSlovimCompilationTarget >> hasWindowsExceptions [
	^ slvmCompilationTarget hasWindowsExceptions
]

{ #category : #initialization }
MbndSlovimCompilationTarget >> initialize [
	super initialize.
	self initializeTypeSystem.
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> initializeWithWBTPlatform: wbtPlatform [
	targetName := wbtPlatform slvmTargetName.
	^ self slvmCompilationTarget: wbtPlatform createSLVMCompilationTarget
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> int64Alignment [
	^ slvmCompilationTarget int64 alignment
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isARM32 [
	^ self wbtArchitecture isARM32
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isARM64 [
	^ self wbtArchitecture isARM64
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isAndroid [
	^ self wbtPlatform isAndroid
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isAppleFamily [
	^ self wbtPlatform isAppleFamily
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isBSD [
	^ self wbtPlatform isBSD
]

{ #category : #testing }
MbndSlovimCompilationTarget >> isBigEndian [
	^ slvmCompilationTarget isBigEndian
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isEmbeddedPAL [
	^ slvmCompilationTarget isEmbeddedPAL
]

{ #category : #converting }
MbndSlovimCompilationTarget >> isEmittingDebugInformation [
	^ slvmCompilationTarget emittingDebugInformation
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isFreeBSD [
	^ self wbtPlatform isFreeBSD
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isGPU [
	^ slvmCompilationTarget isGPU
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isLinux [
	^ self wbtPlatform isLinux
]

{ #category : #testing }
MbndSlovimCompilationTarget >> isLittleEndian [
	^ slvmCompilationTarget isLittleEndian
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isMIPS [
	^ self wbtArchitecture isMIPS
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isMIPSEL [
	^ self wbtArchitecture isMIPSEL
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isMicrosoftXboxOne [
	^ self wbtPlatform isMicrosoftXboxOne
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isMicrosoftXboxSeriesOne [
	^ self wbtPlatform isMicrosoftXboxSeriesOne
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isNintendo3DS [
	^ self wbtPlatform isNintendo3DS
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isNintendoSwitch [
	^ self wbtPlatform isNintendoSwitch
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isNoOperatingSystem [
	^ self wbtPlatform isNoOperatingSystem
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isNullOS [
	^ self wbtPlatform isNullOS
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isOSX [
	^ self wbtPlatform  isOSX
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isSonyPS4 [
	^ self wbtPlatform isSonyPS4
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isSonyPS5 [
	^ self wbtPlatform isSonyPS5
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isUniversalWindowsPlatform [
	^ self wbtPlatform isUniversalWindowsPlatform
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isUnixFamily [
	^ self wbtPlatform isUnixFamily
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isWebAssembly [
	^ self wbtPlatform isWebAssembly
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isWindows [
	^ self wbtPlatform isWindows
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isWindowsFamily [
	^ self wbtPlatform isWindowsFamily
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isWindowsStore [
	^ self wbtPlatform isWindowsStore
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isX86_32 [
	^ self wbtArchitecture isX86_32
]

{ #category : #'platform testing' }
MbndSlovimCompilationTarget >> isX86_64 [
	^ self wbtArchitecture isX86_64
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget >> landingPadSelectorRegisterIndex [
	^ slvmCompilationTarget landingPadSelectorRegisterIndex
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget >> landingPadStackPointerRegisterIndex [
	^ slvmCompilationTarget landingPadStackPointerRegisterIndex
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget >> landingPadStructurePointerRegisterIndex [
	^ slvmCompilationTarget landingPadStructurePointerRegisterIndex
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> machineAttributes [
	^ slvmCompilationTarget machineAttributes
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> machineAttributes: machineAttributes [
	slvmCompilationTarget machineAttributes: machineAttributes
]

{ #category : #'instance creation' }
MbndSlovimCompilationTarget >> newCodeModuleGenerator [
	^ MbndSlovimCodeModuleGenerator new
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> operatingSystemName [
	^ slvmCompilationTarget operatingSystemName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> operatingSystemName: osName [
	slvmCompilationTarget operatingSystemName: osName
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> pointerAlignment [
	^ slvmCompilationTarget pointerAlignment
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> pointerSize [
	^ slvmCompilationTarget pointerSize
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> selectDefaultExceptionModel [
	slvmCompilationTarget selectDefaultExceptionModel.
	(self isLinux or: [ self isOSX ]) ifTrue: [ ^ self ].
	self setNoExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> setForEmbeddedPAL [
	slvmCompilationTarget setForEmbeddedPAL.
	self selectDefaultExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> setForLinux [
	slvmCompilationTarget setForLinux.
	self selectDefaultExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> setForNullOS [
	self slvmCompilationTarget setForNullOS.
	self selectDefaultExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> setForOSX [
	self slvmCompilationTarget setForOSX.
	self selectDefaultExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> setForWindows [
	self slvmCompilationTarget setForWindows.
	self selectDefaultExceptionModel
]

{ #category : #initialization }
MbndSlovimCompilationTarget >> setNoExceptionModel [
	slvmCompilationTarget setNoExceptionModel
]

{ #category : #asserting }
MbndSlovimCompilationTarget >> shouldDisableSLVMOptimizations [
	^ slvmCompilationTarget isSAsmCompilationTarget not
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> slvmCompilationTarget [
	^ slvmCompilationTarget
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> slvmCompilationTarget: anObject [
	slvmCompilationTarget := anObject.
	slvmCompilationTarget objectModel: (MbndSlovimObjectModel new
		managedObjectModel: self managedObjectModel;
		target: slvmCompilationTarget;
		yourself).
	self addSysmelLLVMGCPlugin.
	self selectDefaultExceptionModel
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> slvmCompilationTargetNamed: theTargetName [
	targetName := theTargetName.
	^ self slvmCompilationTarget: (SLVMCompilationTarget perform: targetName asSymbol)
]

{ #category : #'as yet unclassified' }
MbndSlovimCompilationTarget >> supportsDirectThreadLocalStorage [
	^ #(linux windows osx) includes: self operatingSystemName
]

{ #category : #testing }
MbndSlovimCompilationTarget >> supportsLocalAddressForThisContext [
	<compileTime>
	^ slvmCompilationTarget supportsLocalAddressForThisContext
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> systemVendor [
	^ slvmCompilationTarget systemVendor
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> systemVendor: vendor [
	slvmCompilationTarget systemVendor: vendor
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> wbtArchitecture [
	^ self wbtPlatform architecture
]

{ #category : #accessing }
MbndSlovimCompilationTarget >> wbtPlatform [
	^ slvmCompilationTarget wbtPlatform
]

{ #category : #initialization }
MbndSlovimCompilationTarget >> withDebugInformation [
	slvmCompilationTarget withDebugInformation
]

{ #category : #initialization }
MbndSlovimCompilationTarget >> withPositionIndependentCode [
	slvmCompilationTarget withPositionIndependentCode.
]
