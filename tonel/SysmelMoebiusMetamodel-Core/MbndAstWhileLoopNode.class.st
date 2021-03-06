Class {
	#name : #MbndAstWhileLoopNode,
	#superclass : #MbndAstStatementNode,
	#instVars : [
		'condition',
		'action',
		'continueAction'
	],
	#category : #'SysmelMoebiusMetamodel-Core-AST'
}

{ #category : #visiting }
MbndAstWhileLoopNode >> accept: aVisitor [
	^ aVisitor visitWhileLoopNode: self
]

{ #category : #accessing }
MbndAstWhileLoopNode >> action [
	^ action
]

{ #category : #accessing }
MbndAstWhileLoopNode >> action: anObject [
	action := anObject
]

{ #category : #accessing }
MbndAstWhileLoopNode >> condition [
	^ condition
]

{ #category : #accessing }
MbndAstWhileLoopNode >> condition: anObject [
	condition := anObject
]

{ #category : #accessing }
MbndAstWhileLoopNode >> continueAction [
	^ continueAction
]

{ #category : #accessing }
MbndAstWhileLoopNode >> continueAction: anObject [
	continueAction := anObject
]

{ #category : #serialization }
MbndAstWhileLoopNode >> encodeFieldsForMessagePackWith: graphEncoder [
	^ {
		#condition . condition encodeGraphForMessagePackWith: graphEncoder.
		#action . action encodeGraphForMessagePackWith: graphEncoder.
		#continueAction . continueAction encodeGraphForMessagePackWith: graphEncoder.
	} , (super encodeFieldsForMessagePackWith: graphEncoder)
]

{ #category : #testing }
MbndAstWhileLoopNode >> isWhileLoopNode [
	^ true
]

{ #category : #serialization }
MbndAstWhileLoopNode >> materializeWithGraphDecoder: graphDecoder withFields: encodedFields [
	super materializeWithGraphDecoder: graphDecoder withFields: encodedFields.
	condition := encodedFields at: #condition.
	action := encodedFields at: #action.
	continueAction := encodedFields at: #continueAction.
]
