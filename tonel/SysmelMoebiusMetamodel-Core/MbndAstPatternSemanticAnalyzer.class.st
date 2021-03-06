Class {
	#name : #MbndAstPatternSemanticAnalyzer,
	#superclass : #MbndAstVisitor,
	#instVars : [
		'semanticAnalyzer',
		'patternValueType'
	],
	#category : #'SysmelMoebiusMetamodel-Core-SemanticAnalysis'
}

{ #category : #adding }
MbndAstPatternSemanticAnalyzer >> addSameDecorationsOfPatternValueType: type [
	patternValueType isReferenceType ifTrue: [
		^ (type withDecorations: patternValueType baseType decorations) refFor: patternValueType addressSpace
	].

	patternValueType isTemporaryReferenceType ifTrue: [ 
		^ (type withDecorations: patternValueType baseType decorations) tempRefFor: patternValueType addressSpace
	].

	^ type withDecorations: patternValueType decorations
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> patternConcreteValueType [
	^ patternValueType withoutReferences withoutTopDecorations
]

{ #category : #accessing }
MbndAstPatternSemanticAnalyzer >> patternValueType [
	^ patternValueType
]

{ #category : #accessing }
MbndAstPatternSemanticAnalyzer >> patternValueType: anObject [
	patternValueType := anObject
]

{ #category : #accessing }
MbndAstPatternSemanticAnalyzer >> semanticAnalyzer [
	^ semanticAnalyzer
]

{ #category : #accessing }
MbndAstPatternSemanticAnalyzer >> semanticAnalyzer: anObject [
	semanticAnalyzer := anObject
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingAnyPatternNode: node [
	^ node copy
		matchedType: patternValueType;
		yourself
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingBindingPatternNode: node [
	| result |
	result := node copy.

	result requiredType ifNil: [ 
		result requiredType: patternValueType.		
	] ifNotNil: [ :requiredTypeNode |
		result requiredType: (semanticAnalyzer evaluateTypeExpression: requiredTypeNode).
		(result requiredType = patternValueType or: [ result requiredType = patternValueType withoutReferences withoutTopDecorations ]) ifFalse: [
			patternValueType withoutReferences withoutTopDecorations isVariantUnionType ifTrue: [
				| subPattern |
				subPattern := result name isMbndAnonymousSymbol
					ifTrue: [ result valuePattern ]
					ifFalse: [ (result copy requiredType: nil; yourself)].
				^ self visitNode: (MbndAstPatternMatchingVariantTypeSelectionPatternNode new
					position: node;
					expectedValueType: result requiredType;
					valuePattern: subPattern;
					yourself)
			].
		
			^ node asNeverPatternNode
		].
	].

	result valuePattern ifNotNil: [:pattern |
		result valuePattern: (semanticAnalyzer analyzePattern: pattern matchingWithValueOfType: result requiredType)
	].
 
	result name ifNotNil: [
		result
			initialValueKind: (patternValueType isTemporaryReferenceType ifTrue: [ #move ] ifFalse: [ #copy]);
			binding: (semanticAnalyzer createLocalVariableNamed: node name withValueType: result requiredType type: patternValueType at: result).
	].

	result matchedType: patternValueType.
	^ result
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingIntervalPatternNode: node [
	| startValue stopValue |
	startValue := semanticAnalyzer attemptImplicitCast: node start intoType: patternValueType.
	startValue ifNil: [ ^ node asNeverPatternNode ].

	stopValue := semanticAnalyzer attemptImplicitCast: node stop intoType: patternValueType.
	stopValue ifNil: [ ^ node asNeverPatternNode ].

	^ node copy
		matchedType: patternValueType;
		start: startValue;
		stop: stopValue;
		yourself
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingSameValuePatternNode: node [
	| expectedValue matchedValue testingExpression builder |
	expectedValue := semanticAnalyzer attemptImplicitCast: node expectedValue intoType: self patternConcreteValueType.
	expectedValue ifNil: [ ^ node asNeverPatternNode ].
	
	matchedValue := MbndAstPatternMatchingMatchedValueNode new
		position: node;
		type: patternValueType;
		yourself. 
	builder := semanticAnalyzer astBuilder. 
	testingExpression := semanticAnalyzer visitNode: (
		builder unexpandedSend: (builder literalSymbol: #=)
			to: matchedValue withArguments: { expectedValue }
	).
	
	expectedValue isLiteralValueNode ifTrue: [ ^ MbndAstPatternMatchingLiteralValuePatternNode new
		matchedType: patternValueType;
		matchedValue: matchedValue;
		testingExpression: testingExpression;
		value: expectedValue;
		yourself
	].
	
	^ node copy
		matchedType: patternValueType;
		matchedValue: matchedValue;
		testingExpression: testingExpression;
		expectedValue: expectedValue;
		yourself
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingSetPatternNode: node [
	| elements |
	elements := node elements collect: [ :each |
		(semanticAnalyzer attemptImplicitCast: each intoType: patternValueType) ifNil: [ ^ node asNeverPatternNode]
	].

	^ node copy
		matchedType: patternValueType;
		elements: elements;
		yourself
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingTuplePatternNode: node [
	| expectedElementsPatterns patternConcreteValueType |
	patternConcreteValueType := self patternConcreteValueType.
	patternConcreteValueType isTupleType ifFalse: [ ^ node asNeverPatternNode ].
	patternConcreteValueType elements size = node expectedElements size ifFalse: [ ^ node asNeverPatternNode ].
	
	expectedElementsPatterns := patternConcreteValueType elements collectWithIndex: [ :expectedElementType :index |
		semanticAnalyzer analyzePattern: (node expectedElements at: index) matchingWithValueOfType: (self addSameDecorationsOfPatternValueType: expectedElementType )
	].

	"Patterns that are never satisfied are completely removed."
	(expectedElementsPatterns anySatisfy: [ :each | each isNeverPattern ]) ifTrue: [ ^ node asNeverPatternNode ].
	^ node copy
		matchedType: patternValueType;
		expectedElements: expectedElementsPatterns;
		yourself
]

{ #category : #visiting }
MbndAstPatternSemanticAnalyzer >> visitPatternMatchingVariantTypeSelectionPatternNode: node [
	| result bestVariantTypeSelector variantUnionType matchedElementReferenceType |
	result := node copy.
	self assert: result expectedValueType isNotNil.
	variantUnionType := patternValueType withoutReferences withoutTopDecorations.
	self assert: variantUnionType isVariantUnionType.
	bestVariantTypeSelector := variantUnionType findBestSuitableValueTypeFor: result expectedValueType node: node.
	bestVariantTypeSelector size > 1 ifTrue: [ 
		self error: 'Variant pattern matches multiple types {1}.' format: { bestVariantTypeSelector asString } at: node.
	].

	result expectedValueType: bestVariantTypeSelector first.
	result variantTypeSelector: (variantUnionType getTypeSelectorFor: result expectedValueType).
	result variantTypeSelector ifNil: [
		self error: 'Variant pattern with type {1} is never matches.' format: { result expectedValueType asString } at: node.
	].

	matchedElementReferenceType := result expectedValueType.
	patternValueType withoutReferences isConstantType ifTrue: [ matchedElementReferenceType := matchedElementReferenceType const ].
	patternValueType isReferenceType ifTrue: [
		matchedElementReferenceType := matchedElementReferenceType refFor: patternValueType addressSpace
	] ifFalse: [
		matchedElementReferenceType := matchedElementReferenceType ref
	].

	result valuePattern ifNotNil: [:pattern |
		result valuePattern: (semanticAnalyzer analyzePattern: pattern matchingWithValueOfType: matchedElementReferenceType)
	].

	result matchedType: patternValueType.
	^ result
]
