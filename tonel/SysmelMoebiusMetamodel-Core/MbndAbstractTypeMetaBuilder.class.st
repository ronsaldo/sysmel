Class {
	#name : #MbndAbstractTypeMetaBuilder,
	#superclass : #MbndAbstractNamedMetaBuilder,
	#instVars : [
		'typeDefinitionNode',
		'typeDeclarationNode',
		'superTypeNode'
	],
	#category : #'SysmelMoebiusMetamodel-Core-MetaBuilder'
}

{ #category : #'semantic analysis' }
MbndAbstractTypeMetaBuilder >> concretizeNodeAt: node withAnalyzer: analyzer [
	self ensureTypeDeclarationNode.
	self assert: typeDeclarationNode isNotNil.
	
	^ analyzer visitNode: (typeDefinitionNode
		ifNotNil: [ typeDefinitionNode ]
		ifNil: [ typeDeclarationNode ])
]

{ #category : #accessing }
MbndAbstractTypeMetaBuilder >> definition: newBodyNode [
	self ensureTypeDefinitionNode.
	
	typeDefinitionNode body: newBodyNode
]

{ #category : #'semantic analysis' }
MbndAbstractTypeMetaBuilder >> ensureTypeDeclarationNode [
	typeDeclarationNode ifNotNil: [ ^ self ].
	
	typeDeclarationNode := self typeDeclarationNodeClass new
		flags: flags;
		position: position;
		name: name;
		category: metaBuilderScope activePackageName;
		yourself
]

{ #category : #'semantic analysis' }
MbndAbstractTypeMetaBuilder >> ensureTypeDefinitionNode [
	typeDefinitionNode ifNotNil: [ ^ self ].
	self ensureTypeDeclarationNode.
	
	typeDefinitionNode := self typeDefinitionNodeClass new
		position: position;
		declaration: typeDeclarationNode;
		superTypeNode: superTypeNode;
		yourself
]

{ #category : #accessing }
MbndAbstractTypeMetaBuilder >> extend: newBodyNode [
	self ensureTypeDefinitionNode.
	self halt
]

{ #category : #accessing }
MbndAbstractTypeMetaBuilder >> poolDictionaries: poolDictionariesNode [
	self ensureTypeDefinitionNode.
	typeDefinitionNode poolDictionaries: poolDictionariesNode
]

{ #category : #'semantic analysis' }
MbndAbstractTypeMetaBuilder >> typeDeclarationNodeClass [
	self subclassResponsibility
]

{ #category : #'semantic analysis' }
MbndAbstractTypeMetaBuilder >> typeDefinitionNodeClass [
	self subclassResponsibility
]

{ #category : #accessing }
MbndAbstractTypeMetaBuilder >> usedTraits: usedTraitsNode [
	self ensureTypeDefinitionNode.
	typeDefinitionNode usedTraits: usedTraitsNode
]
