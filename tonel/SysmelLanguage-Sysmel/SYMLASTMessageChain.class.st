Class {
	#name : #SYMLASTMessageChain,
	#superclass : #SYMLASTNode,
	#instVars : [
		'receiver',
		'messages'
	],
	#category : #'SysmelLanguage-Sysmel-AST'
}

{ #category : #converting }
SYMLASTMessageChain >> buildMoebiusAST: builder [
	^ (builder messageChainReceiver: (receiver buildMoebiusAST: builder)
		messages: (messages collect: [:message | message buildMoebiusMessageChainMessageAST: builder]))
		position: self;
		yourself
]

{ #category : #accessing }
SYMLASTMessageChain >> children [
	^ { receiver . #messages -> messages }
]

{ #category : #accessing }
SYMLASTMessageChain >> messages [
	^ messages
]

{ #category : #accessing }
SYMLASTMessageChain >> messages: anObject [
	messages := anObject
]

{ #category : #printing }
SYMLASTMessageChain >> printOn: aStream [
	aStream nextPutAll: 'chain'
]

{ #category : #accessing }
SYMLASTMessageChain >> receiver [
	^ receiver
]

{ #category : #accessing }
SYMLASTMessageChain >> receiver: anObject [
	receiver := anObject
]