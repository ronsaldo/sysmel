"
A MbndSysmelLanguageTest is a test class for testing the behavior of MbndSysmelLanguage
"
Class {
	#name : #MbndSysmelLanguageCompileTimeEvaluationTest,
	#superclass : #MbndSysmelLanguageTestCommon,
	#category : #'SysmelLanguage-Sysmel-Tests-Parser'
}

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testBitInvert [
	self assert: (self evaluateSource: '1 bitInvert') value equals: 1 bitInvert.
	self assert: (self evaluateSource: '~1') value equals: 1 bitInvert.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testBlockClosureValueWithArguments [
	| function |
	function := self evaluateSource: '
{:(Int32)x :: Int32 | x + 42}
'.

	self assert: (function valueWithArguments: { 0 }) unwrapMbndValue equals: 42.
	self assert: (function valueWithArguments: { 1 }) unwrapMbndValue equals: 43.
	self assert: (function valueWithArguments: { 2 }) unwrapMbndValue equals: 44.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testBooleanLiteralMessages [
	self assert: (self evaluateSource: 'true and: false') value equals: false.
	self assert: (self evaluateSource: 'true and: false') type isLiteralBooleanFalseType.
	self assert: (self evaluateSource: 'true or: false') value equals: true.
	self assert: (self evaluateSource: 'true or: false') type isLiteralBooleanTrueType.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testBooleanLiterals [
	self assert: (self evaluateSource: 'true') value equals: true.
	self assert: (self evaluateSource: 'true') type isLiteralBooleanTrueType.
	self assert: (self evaluateSource: 'false') value equals: false.
	self assert: (self evaluateSource: 'false') type isLiteralBooleanFalseType.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFieldCompileTimeReflection [
	self evaluateSource: '
struct TestStruct definition: { 
	field a type: Int32.
	field b type: Int32.
}.
'.

	self assert: (self evaluateSource: 'TestStruct allFields size') unwrapMbndValue equals: 2.
	self assert: ((self evaluateSource: 'TestStruct allFields collect: {:(_CompilerObjectType)each :: _CompilerObjectType | each name}')
		unwrapMbndValue collect: #unwrapMbndValue) equals: #(a b).
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFieldCompileTimeReflection2 [
	self evaluateSource: '
struct TestStruct definition: { 
	field a type: Int32.
	field b type: Int32.
}.
'.

	self assert: (self evaluateSource: 'TestStruct allFields size') unwrapMbndValue equals: 2.
	self assert: ((self evaluateSource: 'TestStruct allFields collect: {:each | each name}')
		unwrapMbndValue collect: #unwrapMbndValue) equals: #(a b).
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFunctionNotInCompileTime [
	self should: [ self evaluateSource: '
		function notInCompileTime() => Void := { 
			<notInCompileTime>
		}.
		notInCompileTime()
	'
	] raise: MbndNotInCompileTimeError
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFunctionNotInCompileTimeTransitivity [
	| caller |
	caller := self evaluateSource: '
		function notInCompileTime() => Void := { 
			<notInCompileTime>
		}.

		function caller() => Void := { 
			notInCompileTime().
		}.
	'.
	self deny: caller unwrapMbndValue canBeEvaluatedInCompileTime
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFunctionValueWithArguments [
	| function |
	function := self evaluateSource: '
function f(x: Int32) => Int64
	:= x + 42.
'.

	self assert: (function valueWithArguments: { 0 }) unwrapMbndValue equals: 42.
	self assert: (function valueWithArguments: { 1 }) unwrapMbndValue equals: 43.
	self assert: (function valueWithArguments: { 2 }) unwrapMbndValue equals: 44.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testFunctorTemplate [
	| functionType |
	functionType := self evaluateSource: '
template FunctorTemplate(F: _FunctionType ref)
:= class definition: {
	compileTime constant FunctionType := F typeOfExpression.
}.

function f(x: Float32) => Float32 := x.
FunctorTemplate(f) FunctionType
'.

	self assert: functionType type isMetaType.
	self deny: functionType value isTrait.
	self assert: functionType value isReferenceType.
	functionType := functionType value baseType.

	self deny: functionType isTrait.
	self assert: functionType isFunctionType.
	self assert: functionType argumentTypes size equals: 1.
	self assert: functionType argumentTypes first isPrimitiveFloatType.
	self assert: functionType returnType isPrimitiveFloatType.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testLet [
	self assert: (self evaluateSource: 'let x := 5. x') value equals: 5.
	self assert: (self evaluateSource: 'let x mutable := 5. x') type isReferenceType.
	self assert: (self evaluateSource: 'let x mutable := 5. x') value isInteger not.
	self assert: (self evaluateSource: 'let x mutable type: Int32 := 5. x') type isReferenceType.
	self assert: (self evaluateSource: 'let x mutable type: Int32 := 5. x') value isInteger not.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testLiteralType [
	self assert: (self evaluateSource: '_LiteralInteger') type isMetaType.
	self assert: (self evaluateSource: '_LiteralInteger') value equals: semanticModule compilationTarget literalIntegerType.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testLogicalNot [
	self assert: (self evaluateSource: 'true not') value equals: false.
	self assert: (self evaluateSource: 'false not') value equals: true.
	self assert: (self evaluateSource: '!true') value equals: false.
	self assert: (self evaluateSource: '!false') value equals: true.

	self evaluateSource: '
	function f(v: Boolean8) => Boolean8 := v not.
	function g(v: Boolean8) => Boolean8 := !v.
'.

	self assert: (self evaluateSource: 'f(false)') value equals: true.
	self assert: (self evaluateSource: 'f(true)') value equals: false.
	self assert: (self evaluateSource: 'g(false)') value equals: true.
	self assert: (self evaluateSource: 'g(true)') value equals: false.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixColumnMajorIndices [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3 columnMajorIndices'.

	self assert: matrixValue asRowMajorArray equals: #(
		0 3 6
		1 4 7
		2 5 8
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixIdentity [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3 identity'.

	self assert: matrixValue asRowMajorArray equals: #(
		1 0 0
		0 1 0
		0 0 1
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixOnes [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3 ones'.

	self assert: matrixValue asRowMajorArray equals: #(
		1 1 1
		1 1 1
		1 1 1
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixRowMajorIndices [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3 rowMajorIndices'.

	self assert: matrixValue asRowMajorArray equals: #(
		0 1 2
		3 4 5
		6 7 8
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixScale [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3(2)'.

	self assert: matrixValue asRowMajorArray equals: #(
		2 0 0
		0 2 0
		0 0 2
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMatrixZeros [
	| matrixValue |
	matrixValue := self evaluateSource: 'Float32x3x3 zeros'.

	self assert: matrixValue asRowMajorArray equals: #(
		0 0 0
		0 0 0
		0 0 0
	)
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMaxFunction [
	self evaluateSource: '
function max(a: Int32, b: Int32) => Int32
	:= if: a >= b then: a else: b.
'.

	self assert: (self evaluateSource: 'max(2, 3).') value equals: 3.
	self assert: (self evaluateSource: 'max(2, 3).') type isPrimitiveIntegerType.

	self assert: (self evaluateSource: 'max(3, 2).') value equals: 3.
	self assert: (self evaluateSource: 'max(3, 2).') type isPrimitiveIntegerType.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMessageNotInCompileTime [
	self should: [ self evaluateSource: '
		struct testStruct definition: {
			method notInCompileTime => Void := { 
				<notInCompileTime>
			}.
		}.
		testStruct newValue notInCompileTime.
	'
	] raise: MbndNotInCompileTimeError
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMessageNotInCompileTimeTransitivity [
	| structValue |
	structValue := self evaluateSource: '
		struct testStruct definition: {
			method notInCompileTime => Void := { 
				<notInCompileTime>
			}.
			method caller => Void
				:= self notInCompileTime.
		}.
	'.
	
	self deny: (structValue unwrapMbndValue lookupOverloadedSelector: #caller) canBeEvaluatedInCompileTime
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testMinFunction [
	self evaluateSource: '
function min(a: Int32, b: Int32) => Int32
	:= if: a <= b then: a else: b.
'.

	self assert: (self evaluateSource: 'min(2, 3).') value equals: 2.
	self assert: (self evaluateSource: 'min(2, 3).') type isPrimitiveIntegerType.

	self assert: (self evaluateSource: 'min(3, 2).') value equals: 2.
	self assert: (self evaluateSource: 'min(3, 2).') type isPrimitiveIntegerType.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testNilLiteral [
	self assert: (self evaluateSource: 'nil') value equals: nil.
	self assert: (self evaluateSource: 'nil') type isUndefinedType.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testRecursiveCompileTimeConstant [
	self should: [ self evaluateSource: 'compileTimeConstant X := X. X.' ] raise: Error.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testSequence [
	self assert: (self evaluateSource: '{1 + 2}. 5') value equals: 5.
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testShaderSignatureMetadataExtraction [
	| descriptorSets pushConstants pushConstantsSize |
	self evaluateSource: '
struct SamplingState definition: {
    public field sampler type: GPUSampler.
}.

struct TransformationState definition: {
    public field projectionMatrix type: Float32x4x4.
}.

struct TransformationStateBinding definition: {
    public field state type: TransformationState uniformBufferPointer.
}.

struct TexturingStateBinding definition: {
    public field texture type: GPUTexture2D.
}.

struct PushConstants definition: {
    public field activeMatrix type: UInt32.
}.

class CanvasMesh2DShaderSignature definition: {
    public field texturingState type: TexturingStateBinding bindingSetPointer; location: 2.
    public field samplingState type: SamplingState bindingSetPointer; location: 0.
    public field transformationStateBinding type: TransformationStateBinding bindingSetPointer; location: 1.
    public field pushConstants type: TexturingStateBinding pushConstantPointer.

    macro method transformationState := ``(`,self transformationStateBinding _ state _).
}.
'.

	descriptorSets := (self evaluateSource: '(CanvasMesh2DShaderSignature allFields select: {:each | each isDescriptorSetBinding}) sorted: {:a :b | a asDescriptorSetIndex < b asDescriptorSetIndex}') unwrapMbndValue.
	pushConstants := (self evaluateSource: 'CanvasMesh2DShaderSignature allFields select: {:each | each isPushConstantBinding}') unwrapMbndValue.
	pushConstantsSize := (self evaluateSource: '(CanvasMesh2DShaderSignature allFields select: {:each | each isPushConstantBinding} thenCollect: {:each | each valueType instanceSize}) sum') unwrapMbndValue.

	self assert: descriptorSets size equals: 3.
	self assert: descriptorSets first isFieldVariable.
	self assert: descriptorSets first name equals: #samplingState.

	self assert: descriptorSets second isFieldVariable.
	self assert: descriptorSets second name equals: #transformationStateBinding.

	self assert: descriptorSets third isFieldVariable.
	self assert: descriptorSets third name equals: #texturingState.

	self assert: pushConstants size equals: 1.
	self assert: pushConstants first name equals: #pushConstants.
	self assert: pushConstantsSize equals: 4.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testSignExtension [
	self evaluateSource: '
function f(v: Int8) => Int64
	:= v.
'.

	self assert: (self evaluateSource: 'f(0).') value equals: 0.
	self assert: (self evaluateSource: 'f(1).') value equals: 1.
	self assert: (self evaluateSource: 'f(10).') value equals: 10.
	self assert: (self evaluateSource: 'f(127).') value equals: 127.
	self assert: (self evaluateSource: 'f(-10).') value equals: -10.
	self assert: (self evaluateSource: 'f(-1).') value equals: -1.
	self assert: (self evaluateSource: 'f(-127).') value equals: -127.
	self assert: (self evaluateSource: 'f(-128).') value equals: -128.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testTruncateSigned [
	self evaluateSource: '
function f(v: Int32) => Int8
	:= v castTo: Int8.
'.

	self assert: (self evaluateSource: 'f(16r7f).') value equals: 16r7f.
	self assert: (self evaluateSource: 'f(-1).') value equals: -1.
	self assert: (self evaluateSource: 'f(-128).') value equals: -128.
	self assert: (self evaluateSource: 'f(-20000).') value equals: -32.
	self assert: (self evaluateSource: 'f(-12345678).') value equals: -78.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testTruncateUnsigned [
	self evaluateSource: '
function f(v: UInt32) => UInt8
	:= v castTo: UInt8.
'.

	self assert: (self evaluateSource: 'f(16rff).') value equals: 16rff.
	self assert: (self evaluateSource: 'f(16rffff).') value equals: 16rff.
	self assert: (self evaluateSource: 'f(16rffffff).') value equals: 16rff.
	self assert: (self evaluateSource: 'f(16rffffffff).') value equals: 16rff.

]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testVectorConstruction [
	self assert: (self evaluateSource: '1.0 castTo: Float32x2') slots equals: #(1.0 1.0).
	self assert: (self evaluateSource: 'Float32x2(1.0, 2.0)') slots equals: #(1.0 2.0).
	self assert: (self evaluateSource: 'Float32x2 x: 1.0 y: 2.0') slots equals: #(1.0 2.0).
	self assert: (self evaluateSource: 'Float32x3(Float32x2(1.0, 2.0), 3.0)') slots equals: #(1.0 2.0 3.0).
	self assert: (self evaluateSource: 'Float32x3(1.0, Float32x2(2.0, 3.0))') slots equals: #(1.0 2.0 3.0).
]

{ #category : #tests }
MbndSysmelLanguageCompileTimeEvaluationTest >> testZeroExtension [
	self evaluateSource: '
function f(v: UInt8) => UInt64
	:= v.
'.

	self assert: (self evaluateSource: 'f(0).') value equals: 0.
	self assert: (self evaluateSource: 'f(1).') value equals: 1.
	self assert: (self evaluateSource: 'f(10).') value equals: 10.
	self assert: (self evaluateSource: 'f(255).') value equals: 255.

]
