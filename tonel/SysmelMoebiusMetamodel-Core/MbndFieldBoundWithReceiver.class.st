Class {
	#name : #MbndFieldBoundWithReceiver,
	#superclass : #MbndImplicitReceiverBoundSymbol,
	#instVars : [
		'field'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Variables'
}

{ #category : #'semantic analysis' }
MbndFieldBoundWithReceiver >> analyzeReferenceNode: referenceNode with: analyzer [
	| receiverReference receiverReferenceType getterClass getterNode |
	receiverReference := receiver analyzeReferenceNode: referenceNode with: analyzer.
	receiverReferenceType := receiverReference type withoutTopDecorations.
	getterClass := ((receiverReferenceType isReferenceLikeValueType or: [ receiverReferenceType isTypePassedByReference ]) and: [ field bits isNil ])
		ifTrue: [ MbndAstGetFieldReferenceNode ]
		ifFalse: [ MbndAstGetFieldValueNode ].
		
	getterNode := getterClass new
		position: referenceNode;
		aggregate: receiverReference;
		field: field;
		yourself.

	^ analyzer visitNode: getterNode
]

{ #category : #accessing }
MbndFieldBoundWithReceiver >> field [
	^ field
]

{ #category : #accessing }
MbndFieldBoundWithReceiver >> field: anObject [
	field := anObject
]
