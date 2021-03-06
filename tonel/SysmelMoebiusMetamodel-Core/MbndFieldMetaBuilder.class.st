Class {
	#name : #MbndFieldMetaBuilder,
	#superclass : #MbndAbstractVariableMetaBuilder,
	#traits : 'TMbndEntityWithVisibility',
	#classTraits : 'TMbndEntityWithVisibility classTrait',
	#instVars : [
		'bits',
		'location',
		'intrinsic'
	],
	#category : #'SysmelMoebiusMetamodel-Core-MetaBuilder'
}

{ #category : #accessing }
MbndFieldMetaBuilder >> bits: bitCount [
	bits := bitCount
]

{ #category : #'semantic analysis' }
MbndFieldMetaBuilder >> concretizeNodeAt: node withAnalyzer: analyzer [
	| result |
	result := (analyzer astBuilder
		defineField: name withValueType: valueType withInitialValue: initialValueNode)
		position: position;
		minimalAlignment: alignment;
		bits: bits;
		flags: flags;
		location: location;
		intrinsic: intrinsic;
		yourself.
	^ analyzer visitNode: result
]

{ #category : #accessing }
MbndFieldMetaBuilder >> intrinsic [
	^ intrinsic
]

{ #category : #accessing }
MbndFieldMetaBuilder >> intrinsic: anObject [
	intrinsic := anObject
]

{ #category : #accessing }
MbndFieldMetaBuilder >> location [
	^ location
]

{ #category : #accessing }
MbndFieldMetaBuilder >> location: anObject [
	location := anObject
]
