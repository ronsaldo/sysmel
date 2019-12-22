Class {
	#name : #MbndRBAstConverter,
	#superclass : #RBProgramNodeVisitor,
	#instVars : [
		'builder',
		'sourceCollection'
	],
	#category : #'SysmelLanguage-Pharo'
}

{ #category : #visiting }
MbndRBAstConverter >> compilationTarget [
	^ builder compilationTarget
]

{ #category : #'as yet unclassified' }
MbndRBAstConverter >> convertAST: ast parsedFromSourceCollection: theSourceCollection withBuilder: astBuilder [
	builder := astBuilder.
	sourceCollection := theSourceCollection.
	^ self visitNode: ast
]

{ #category : #visiting }
MbndRBAstConverter >> convertArgumentNode: argumentNode [
	^ (builder argument: argumentNode name type: nil)
		position: (self sourcePositionFromNode: argumentNode)
]

{ #category : #visiting }
MbndRBAstConverter >> sourcePositionFromNode: node [
	^ MbndSourcePosition new
		collection: sourceCollection;
		start: node start;
		start: node stop;
		yourself
]

{ #category : #visiting }
MbndRBAstConverter >> visitAssignmentNode: assignmentNode [
	| receiver value position selector |
	receiver := self visitNode: assignmentNode variable.
	value := self visitNode: assignmentNode value.
	position := self sourcePositionFromNode: assignmentNode.
	selector := (builder literalSymbol: #assignValue:)
		position: position.
	^ (builder unexpandedSend: selector to: receiver withArguments: { value })
		position: position
]

{ #category : #visiting }
MbndRBAstConverter >> visitBlockNode: blockNode [
	| arguments body |
	arguments := blockNode arguments collect: [ :arg | self convertArgumentNode: arg ].
	body := self visitNode: blockNode body.
	
	^ (builder blockClosure: body arguments: arguments returnType: nil)
		position: (self sourcePositionFromNode: blockNode)
]

{ #category : #visiting }
MbndRBAstConverter >> visitCascadeNode: cascadeNode [
	| receiver messagePosition selector messages |
	receiver := self visitNode: cascadeNode messages first receiver.
	messages := cascadeNode messages collect: [ :message |
		messagePosition := self sourcePositionFromNode: message.
		selector := (builder literalSymbol: message selector)
			position: messagePosition.
		(builder messageChainMessageSelector: selector arguments: (message arguments collect: [:arg |
			self visitNode: arg
		]))
			position: messagePosition
	].
	^ (builder messageChainReceiver: receiver messages: messages)
		position: (self sourcePositionFromNode: cascadeNode)
]

{ #category : #visiting }
MbndRBAstConverter >> visitLiteralArrayNode: node [
	| contents type |
	contents := node value.
	type := node isForByteArray
		ifTrue: [ self compilationTarget literalByteArrayType ]
		ifFalse: [ self compilationTarget literalArrayType ].
	^ (builder literal: contents type: type)
		position: (self sourcePositionFromNode: node).
]

{ #category : #visiting }
MbndRBAstConverter >> visitLiteralNode: node [
	^ (node value asMoebiusLiteralValueNodeWithTarget: self compilationTarget)
		position: (self sourcePositionFromNode: node)
]

{ #category : #visiting }
MbndRBAstConverter >> visitMessageNode: messageNode [ 
	| messagePosition selector |
	self assert: messageNode isCascaded not.

	messagePosition := self sourcePositionFromNode: messageNode.
	selector := (builder literalSymbol: messageNode selector)
		position: messagePosition.
	^ (builder unexpandedSend: selector
		to: (self visitNode: messageNode receiver)
		withArguments: (messageNode arguments collect: [ :arg | self visitNode: arg ]))
		position: messagePosition
]

{ #category : #visiting }
MbndRBAstConverter >> visitReturnNode: returnNode [
	^ (builder return: (self visitNode: returnNode value))
		position: (self sourcePositionFromNode: returnNode)
]

{ #category : #visiting }
MbndRBAstConverter >> visitSequenceNode: sequenceNode [
	| result |
	result := (builder sequence: (sequenceNode statements collect: [:statement | self visitNode: statement]))
		position: (self sourcePositionFromNode: sequenceNode).
	^ result lexicallyScoped
	
]

{ #category : #visiting }
MbndRBAstConverter >> visitVariableNode: node [
	^ (builder identifier: node name)
		position: (self sourcePositionFromNode: node)
]