Class {
	#name : #MbndPointerArithmeticOperationCoercionMacro,
	#superclass : #MbndAbstractMacro,
	#category : #'SysmelMoebiusMetamodel-Core-Macro'
}

{ #category : #'as yet unclassified' }
MbndPointerArithmeticOperationCoercionMacro >> computePointerArithmetic: node coercionTypeOf: leftValueType with: rightValueType [
	leftValueType == rightValueType ifTrue: [ ^ leftValueType ].
	leftValueType isUndefinedType ifTrue: [ ^ rightValueType withoutReferences ].
	rightValueType isUndefinedType ifTrue: [ ^ leftValueType withoutReferences ].
	
	self halt
]

{ #category : #'as yet unclassified' }
MbndPointerArithmeticOperationCoercionMacro >> expandMessageNode: node with: analyzer [
	| leftAnalyzedNode rightAnalyzedNode leftValueType rightValueType coercionType leftOperand rightOperand |
	self assert: node receiver isNotNil.
	self assert: node arguments size = 1.

	"Analyze the node."
	leftAnalyzedNode := analyzer analyzeNodeIfNeeded: node receiver.
	rightAnalyzedNode := analyzer analyzeNodeIfNeeded: node arguments first.

	"Get the value types of the nodes."		
	leftValueType := leftAnalyzedNode type bestConcreteTypeForLiteralValueNode: leftAnalyzedNode inScope: analyzer scope.
	rightValueType := rightAnalyzedNode type bestConcreteTypeForLiteralValueNode: rightAnalyzedNode inScope: analyzer scope.
	
	"Compute the coercion type."
	coercionType := self computePointerArithmetic: node coercionTypeOf: leftValueType withoutReferences with: rightValueType withoutReferences.

	"Apply the coercion to the operands."
	leftOperand := analyzer addImplicitCastFor: leftAnalyzedNode to: coercionType.
	rightOperand := analyzer addImplicitCastFor: rightAnalyzedNode to: coercionType.
	
	"Create the expanded message send node, and analyze it"
	^ analyzer visitNode: ((analyzer astBuilder
		send: node selector to: leftOperand withArguments: { rightOperand })
		position: node;
		yourself)
]