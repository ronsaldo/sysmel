Class {
	#name : #MbndAstLiteralValueNode,
	#superclass : #MbndAstNode,
	#instVars : [
		'value'
	],
	#category : #'SysmelMoebiusMetamodel-Core-AST'
}

{ #category : #visiting }
MbndAstLiteralValueNode >> accept: aVisitor [
	^ aVisitor visitLiteralValueNode: self
]

{ #category : #'as yet unclassified' }
MbndAstLiteralValueNode >> evaluateMbndValue [
	^ MbndValue value: value type: type
]

{ #category : #testing }
MbndAstLiteralValueNode >> isLiteralSymbolValueNode [
	^ type isLiteralSymbolType
]

{ #category : #testing }
MbndAstLiteralValueNode >> isLiteralValueNode [
	^ true
]

{ #category : #printing }
MbndAstLiteralValueNode >> printValueString [
	^ value printString
]

{ #category : #accessing }
MbndAstLiteralValueNode >> value [
	^ value
]

{ #category : #accessing }
MbndAstLiteralValueNode >> value: anObject [
	value := anObject
]