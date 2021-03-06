Class {
	#name : #MbndSlovimSpirVCodeModuleGeneratorTest,
	#superclass : #MbndSlovimCodeModuleGeneratorCommonTest,
	#instVars : [
		'spirvModule'
	],
	#category : #'SysmelMoebiusCodeGenerator-Slovim-Tests-Module'
}

{ #category : #running }
MbndSlovimSpirVCodeModuleGeneratorTest >> generateSpirV [
	spirvModule := ssaModule asLowLevelModule.
	self flag: 'TODO: use spirv-val, if available.'
]

{ #category : #running }
MbndSlovimSpirVCodeModuleGeneratorTest >> setUp [
	semanticModule := MbndCompilationEnvironment spirv_vulkan coreModule.
	semanticModule name: #TestModule.
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testComputeShaderLocalSize [
	| shaderController shaderImplementation callWithLocalSize |
	self evaluateSysmelSource: '
function calledWithLocalSize(localSize: UInt32x3) => Void := {
}.

function shader computeShader externC() => Void := {
	<localSize: 4>
	calledWithLocalSize(localSize).
}.
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			call
			returnVoid
		))
	).
	self assert: (shaderController propertyAt: #localSizeX) equals: 4.
	self assert: (shaderController propertyAt: #localSizeY) equals: 1.
	self assert: (shaderController propertyAt: #localSizeZ) equals: 1.
	
	shaderImplementation := shaderController basicBlocks first firstInstruction functionValue.
	self assert: shaderImplementation allInstructionsMnemonics equals: #(
		(entry (jump))
		(body (call returnVoid))
	).
	
	callWithLocalSize := shaderImplementation basicBlocks second firstInstruction.
	self assert: callWithLocalSize arguments first isConstantVectorValue.
	self assert: (callWithLocalSize arguments first slots collect: [:each | each value]) equals: #(4 1 1).
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testComputeShaderLocalSize2 [
	| shaderController shaderImplementation callWithLocalSize |
	self evaluateSysmelSource: '
function calledWithLocalSize(localSize: UInt32x3) => Void := {
}.

function shader computeShader externC() => Void := {
	<localSize: #(4 3)>
	calledWithLocalSize(localSize).
}.
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			call
			returnVoid
		))
	).
	self assert: (shaderController propertyAt: #localSizeX) equals: 4.
	self assert: (shaderController propertyAt: #localSizeY) equals: 3.
	self assert: (shaderController propertyAt: #localSizeZ) equals: 1.
	
	shaderImplementation := shaderController basicBlocks first firstInstruction functionValue.
	self assert: shaderImplementation allInstructionsMnemonics equals: #(
		(entry (jump))
		(body (call returnVoid))
	).
	
	callWithLocalSize := shaderImplementation basicBlocks second firstInstruction.
	self assert: callWithLocalSize arguments first isConstantVectorValue.
	self assert: (callWithLocalSize arguments first slots collect: [:each | each value]) equals: #(4 3 1).
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testComputeShaderLocalSize3 [
	| shaderController shaderImplementation callWithLocalSize |
	self evaluateSysmelSource: '
function calledWithLocalSize(localSize: UInt32x3) => Void := {
}.

function shader computeShader externC() => Void := {
	<localSize: #(4 3 7)>
	calledWithLocalSize(localSize).
}.
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			call
			returnVoid
		))
	).
	self assert: (shaderController propertyAt: #localSizeX) equals: 4.
	self assert: (shaderController propertyAt: #localSizeY) equals: 3.
	self assert: (shaderController propertyAt: #localSizeZ) equals: 7.
	
	shaderImplementation := shaderController basicBlocks first firstInstruction functionValue.
	self assert: shaderImplementation allInstructionsMnemonics equals: #(
		(entry (jump))
		(body (call returnVoid))
	).
	
	callWithLocalSize := shaderImplementation basicBlocks second firstInstruction.
	self assert: callWithLocalSize arguments first isConstantVectorValue.
	self assert: (callWithLocalSize arguments first slots collect: [:each | each value]) equals: #(4 3 7).
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testComputeShaderSharedStorage [
	| shaderController shaderImplementation |
	self evaluateSysmelSource: '
class Shader definition:
{ 
	field sharedInt workgroupShared type: UInt32.

	function main computeShader() => Void := {
		<localSize: 1>
		sharedInt := 0
	}.

}.
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #'Shader::main'.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			alloca
			call
			returnVoid
		))
	).
	self assert: (shaderController propertyAt: #localSizeX) equals: 1.
	self assert: (shaderController propertyAt: #localSizeY) equals: 1.
	self assert: (shaderController propertyAt: #localSizeZ) equals: 1.
	
	shaderImplementation := shaderController basicBlocks first firstInstruction nextInstruction functionValue.
	self assert: shaderImplementation allInstructionsMnemonics equals: #(
		(entry (jump))
		(body (store returnVoid))
	).

]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderIO [
	| shaderController |
	self evaluateSysmelSource: '
struct ShaderIO definition: {
	public field color type: Float32x4; location: 0.
}.

function shader fragmentShader externC(stageIn: ShaderIO inputChannel, stageOut: ShaderIO outputChannel) => Void := { 
	stageOut send: stageIn receive
}
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			alloca
			alloca
			load
			getElementPointer
			store
			call
			getElementPointer
			load
			store
			returnVoid
		))
	).
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderNop [
	| shaderController |
	self evaluateSysmelSource: '
function shader fragmentShader externC() => Void := { 
}
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			call
			returnVoid
		))
	).
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderSampleTextureColor [
	| shaderImplementation |
	self evaluateSysmelSource: '
struct Input definition: {
	public field texcoord type: Float32x2; location: 0.
}.

struct Output definition: {
	public field color type: Float32x4; location: 0.
}.

class Samplers definition: {
    public field sampler type: GPUSampler.
}.

class TexturingState definition: {
    public field texture type: GPUTexture2D.
}.

class ShaderInterface definition: {
	field samplerState type: Samplers bindingSetPointer.
	field texturingState type: TexturingState bindingSetPointer.

	function shader fragmentShader externC(stageIn: Input inputChannel, stageOut: Output outputChannel) => Void := { 
		let color := samplerState _ sampler
							sample: texturingState _ texture at: stageIn receive texcoord.
		stageOut send: (Output newValue color: color)
	}
}.

';
	generateSSAModule;
	validateSpirV.
	
	shaderImplementation := ssaModule globalNamed: #'_ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE'.
	self assert: shaderImplementation asFullSExpressionForTesting equals: #(
function _ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE cdecl void
	arguments (
		(self (pointer (struct 'class.TestModule::ShaderInterface') function) ())
		(stageIn (pointer (struct 'struct.TestModule::Input') function) ())
		(stageOut (pointer (struct 'struct.TestModule::Output') function) ())
	)
	literals (
	)
	blocks (
		(()
			(nil jump void () () (block 2 body))
		)
		(()
			(nil load sampler () ((global 1 _ZN10TestModule8Samplers7sampler13set0_binding0E)))
			(nil load texture2D () ((global 2 _ZN10TestModule14TexturingState7texture13set1_binding0E)))
			(nil load (struct 'struct.TestModule::Input') () ((local 1 stageIn)))
			(nil extractValue (vector f32 2) () ((local 6 nil)) (0))
			(nil combineSamplerWithTexture (sampledTexture texture2D) () ((local 4 nil) (local 5 nil)))
			(nil sampleImplicitLOD (vector f32 4) () ((local 8 nil) (local 7 nil)))
			(nil insertValue (struct 'struct.TestModule::Output') () ((constStruct (struct 'struct.TestModule::Output') ((constVector (vector f32 4) ((const f32 0.0) (const f32 0.0) (const f32 0.0) (const f32 0.0))))) (local 9 nil)) (0))
			(nil store void () ((local 10 nil) (local 2 stageOut)))
			(nil returnVoid void () ())
		)
	)

	)
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderSampleTextureColorWithDepth [
	| shaderImplementation |
	self evaluateSysmelSource: '
struct Input definition: {
	public field texcoord type: Float32x2; location: 0.
}.

struct Output definition: {
	public field color type: Float32x4; location: 0.
}.

class Samplers definition: {
    public field sampler type: GPUSampler.
}.

class TexturingState definition: {
    public field texture type: GPUDepthTexture2D.
}.

class ShaderInterface definition: {
	field samplerState type: Samplers bindingSetPointer.
	field texturingState type: TexturingState bindingSetPointer.

	function shader fragmentShader externC(stageIn: Input inputChannel, stageOut: Output outputChannel) => Void := { 
		let color := samplerState _ sampler
							sample: texturingState _ texture at: stageIn receive texcoord depth: 0.0.
		stageOut send: (Output newValue color: color)
	}
}.

';
	generateSSAModule;
	validateSpirV.
	
	shaderImplementation := ssaModule globalNamed: #'_ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE'.
	self assert: shaderImplementation asFullSExpressionForTesting equals: #(
function _ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE cdecl void
	arguments (
		(self (pointer (struct 'class.TestModule::ShaderInterface') function) ())
		(stageIn (pointer (struct 'struct.TestModule::Input') function) ())
		(stageOut (pointer (struct 'struct.TestModule::Output') function) ())
	)
	literals (
	)
	blocks (
		(()
			(nil jump void () () (block 2 body))
		)
		(()
			(nil load sampler () ((global 1 _ZN10TestModule8Samplers7sampler13set0_binding0E)))
			(nil load depthTexture2D () ((global 2 _ZN10TestModule14TexturingState7texture13set1_binding0E)))
			(nil load (struct 'struct.TestModule::Input') () ((local 1 stageIn)))
			(nil extractValue (vector f32 2) () ((local 6 nil)) (0))
			(nil combineSamplerWithTexture (sampledTexture depthTexture2D) () ((local 4 nil) (local 5 nil)))
			(nil sampleDepthRefImplicitLOD f32 () ((local 8 nil) (local 7 nil) (const f32 0.0)))
			(nil makeVector (vector f32 4) () ((local 9 nil) (local 9 nil) (local 9 nil) (local 9 nil)))
			(nil insertValue (struct 'struct.TestModule::Output') () ((constStruct (struct 'struct.TestModule::Output') ((constVector (vector f32 4) ((const f32 0.0) (const f32 0.0) (const f32 0.0) (const f32 0.0))))) (local 10 nil)) (0))
			(nil store void () ((local 11 nil) (local 2 stageOut)))
			(nil returnVoid void () ())
		)
	)
	)
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderSampleTextureColorWithDerivatives [
	| shaderImplementation |
	self evaluateSysmelSource: '
struct Input definition: {
	public field texcoord type: Float32x2; location: 0.
}.

struct Output definition: {
	public field color type: Float32x4; location: 0.
}.

class Samplers definition: {
    public field sampler type: GPUSampler.
}.

class TexturingState definition: {
    public field texture type: GPUTexture2D.
}.

class ShaderInterface definition: {
	field samplerState type: Samplers bindingSetPointer.
	field texturingState type: TexturingState bindingSetPointer.

	function shader fragmentShader externC(stageIn: Input inputChannel, stageOut: Output outputChannel) => Void := { 
		let color := samplerState _ sampler
							sample: texturingState _ texture at: stageIn receive texcoord dtdx: Float32x2(0.0, 0.0) dtdy: Float32x2(0.0, 0.0).
		stageOut send: (Output newValue color: color)
	}
}.

';
	generateSSAModule;
	validateSpirV.
	
	shaderImplementation := ssaModule globalNamed: #'_ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE'.
	self assert: shaderImplementation asFullSExpressionForTesting equals: #(
function _ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE cdecl void
	arguments (
		(self (pointer (struct 'class.TestModule::ShaderInterface') function) ())
		(stageIn (pointer (struct 'struct.TestModule::Input') function) ())
		(stageOut (pointer (struct 'struct.TestModule::Output') function) ())
	)
	literals (
	)
	blocks (
		(()
			(nil jump void () () (block 2 body))
		)
		(()
			(nil load sampler () ((global 1 _ZN10TestModule8Samplers7sampler13set0_binding0E)))
			(nil load texture2D () ((global 2 _ZN10TestModule14TexturingState7texture13set1_binding0E)))
			(nil load (struct 'struct.TestModule::Input') () ((local 1 stageIn)))
			(nil extractValue (vector f32 2) () ((local 6 nil)) (0))
			(nil combineSamplerWithTexture (sampledTexture texture2D) () ((local 4 nil) (local 5 nil)))
			(nil sampleExplicitGradient (vector f32 4) () ((local 8 nil) (local 7 nil) (constVector (vector f32 2) ((const f32 0.0) (const f32 0.0))) (constVector (vector f32 2) ((const f32 0.0) (const f32 0.0)))))
			(nil insertValue (struct 'struct.TestModule::Output') () ((constStruct (struct 'struct.TestModule::Output') ((constVector (vector f32 4) ((const f32 0.0) (const f32 0.0) (const f32 0.0) (const f32 0.0))))) (local 9 nil)) (0))
			(nil store void () ((local 10 nil) (local 2 stageOut)))
			(nil returnVoid void () ())
		)
	)

	)
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderSampleTextureColorWithLod [
	| shaderImplementation |
	self evaluateSysmelSource: '
struct Input definition: {
	public field texcoord type: Float32x2; location: 0.
}.

struct Output definition: {
	public field color type: Float32x4; location: 0.
}.

class Samplers definition: {
    public field sampler type: GPUSampler.
}.

class TexturingState definition: {
    public field texture type: GPUTexture2D.
}.

class ShaderInterface definition: {
	field samplerState type: Samplers bindingSetPointer.
	field texturingState type: TexturingState bindingSetPointer.

	function shader fragmentShader externC(stageIn: Input inputChannel, stageOut: Output outputChannel) => Void := { 
		let color := samplerState _ sampler
							sample: texturingState _ texture at: stageIn receive texcoord lod: 0.0.
		stageOut send: (Output newValue color: color)
	}
}.

';
	generateSSAModule;
	validateSpirV.
	
	shaderImplementation := ssaModule globalNamed: #'_ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE'.
	self assert: shaderImplementation asFullSExpressionForTesting equals: #(
function _ZN10TestModule15ShaderInterface6shader6__implERNS0_E12inputChannelINS_5InputEE13outputChannelINS_6OutputEE cdecl void
	arguments (
		(self (pointer (struct 'class.TestModule::ShaderInterface') function) ())
		(stageIn (pointer (struct 'struct.TestModule::Input') function) ())
		(stageOut (pointer (struct 'struct.TestModule::Output') function) ())
	)
	literals (
	)
	blocks (
		(()
			(nil jump void () () (block 2 body))
		)
		(()
			(nil load sampler () ((global 1 _ZN10TestModule8Samplers7sampler13set0_binding0E)))
			(nil load texture2D () ((global 2 _ZN10TestModule14TexturingState7texture13set1_binding0E)))
			(nil load (struct 'struct.TestModule::Input') () ((local 1 stageIn)))
			(nil extractValue (vector f32 2) () ((local 6 nil)) (0))
			(nil combineSamplerWithTexture (sampledTexture texture2D) () ((local 4 nil) (local 5 nil)))
			(nil sampleExplicitLOD (vector f32 4) () ((local 8 nil) (local 7 nil) (const f32 0.0)))
			(nil insertValue (struct 'struct.TestModule::Output') () ((constStruct (struct 'struct.TestModule::Output') ((constVector (vector f32 4) ((const f32 0.0) (const f32 0.0) (const f32 0.0) (const f32 0.0))))) (local 9 nil)) (0))
			(nil store void () ((local 10 nil) (local 2 stageOut)))
			(nil returnVoid void () ())
		)
	)
	)
]

{ #category : #tests }
MbndSlovimSpirVCodeModuleGeneratorTest >> testFragmentShaderSolidColorIO [
	| shaderController |
	self evaluateSysmelSource: '
struct ShaderIO definition: {
	public field color type: Float32x4; location: 0.
}.

function shader fragmentShader externC(stageOut: ShaderIO outputChannel) => Void := { 
	stageOut send: (ShaderIO newValue color: Float32x4(1.0, 0.0, 0.0, 1.0))
}
';
	generateSSAModule;
	validateSpirV.
	
	shaderController := ssaModule globalNamed: #shader.
	self assert: shaderController allInstructionsMnemonics equals: #(
		(entry (
			alloca
			call
			getElementPointer
			load
			store
			returnVoid
		))
	).
]

{ #category : #running }
MbndSlovimSpirVCodeModuleGeneratorTest >> validateSpirV [
	self generateSpirV
]
