"
I am a sequence of expression. I evaluate all of my elements in sequence, and then I return the value of my last element.
"
Class {
	#name : #MbndAstSequenceNode,
	#superclass : #MbndAstNode,
	#instVars : [
		'pragmas',
		'elements'
	],
	#category : #'SysmelMoebiusMetamodel-Core-AST'
}

{ #category : #visiting }
MbndAstSequenceNode >> accept: aVisitor [
	^ aVisitor visitSequenceNode: self
]

{ #category : #accessing }
MbndAstSequenceNode >> elements [
	^ elements
]

{ #category : #accessing }
MbndAstSequenceNode >> elements: anObject [
	elements := anObject
]

{ #category : #serialization }
MbndAstSequenceNode >> encodeFieldsForMessagePackWith: graphEncoder [
	^ {
		#pragmas . pragmas encodeGraphForMessagePackWith: graphEncoder.
		#elements . elements encodeGraphForMessagePackWith: graphEncoder.
	} , (super encodeFieldsForMessagePackWith: graphEncoder)
]

{ #category : #initialization }
MbndAstSequenceNode >> initialize [
	super initialize.
	pragmas := #().
	
]

{ #category : #testing }
MbndAstSequenceNode >> isSequenceNode [
	^ true
]

{ #category : #'as yet unclassified' }
MbndAstSequenceNode >> markStorageCreationNodeForCopyEllision: node [
	elements ifNotEmpty: [ elements last markStorageCreationNodeForCopyEllision: node ]
]

{ #category : #serialization }
MbndAstSequenceNode >> materializeWithGraphDecoder: graphDecoder withFields: encodedFields [
	super materializeWithGraphDecoder: graphDecoder withFields: encodedFields.
	pragmas := encodedFields at: #pragmas.
	elements := encodedFields at: #elements.
]

{ #category : #accessing }
MbndAstSequenceNode >> pragmaAt: pragmaName ifAbsent: absentBlock ifPresent: presentBlock [
	pragmas do: [:pragma |
		pragma selector = pragmaName ifTrue: [ ^ presentBlock value: pragma ]
	].
	
	^ absentBlock value
]

{ #category : #accessing }
MbndAstSequenceNode >> pragmas [
	^ pragmas
]

{ #category : #accessing }
MbndAstSequenceNode >> pragmas: anObject [
	pragmas := anObject
]

{ #category : #composition }
MbndAstSequenceNode >> withNextExpression: anotherNode [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self copy
		elements: elements , { anotherNode };
		yourself
]
