Class {
	#name : #MbndAbstractTemplate,
	#superclass : #Object,
	#traits : 'TMbndProgramEntity - {#hasPublicVisibility} + TMbndFlagContainer + TMbndEntityWithVisibility',
	#classTraits : 'TMbndProgramEntity classTrait + TMbndFlagContainer classTrait + TMbndEntityWithVisibility classTrait',
	#instVars : [
		'name',
		'parent',
		'definitionFragments',
		'argumentDefinitions',
		'argumentTupleType',
		'children'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Template'
}

{ #category : #serialization }
MbndAbstractTemplate class >> messagePackTypeName [
	^ nil
]

{ #category : #adding }
MbndAbstractTemplate >> addChild: aChild [
	self assert: aChild parent isNil.
	aChild parent: self.
	self children add: aChild.
]

{ #category : #adding }
MbndAbstractTemplate >> addExtension: extensionBody inScope: extensionScope [
	| fragment |
	fragment := MbndTemplateDefinitionFragment new
		node: extensionBody; scope: extensionScope; yourself.
	definitionFragments add: fragment.
	self instances valuesDo: [ :instance |
		fragment applyToInstance: instance
	]
]

{ #category : #'semantic analysis' }
MbndAbstractTemplate >> analyzeAndEvaluateApplicationNode: node withAnalyzer: analyzer [
	| argumentsNode arguments rawArguments rawArgumentsLiteralNode convertedArgumentsNode result |
	argumentsNode := node arguments first.
	rawArguments := analyzer evaluateCompileTimeExpression: argumentsNode.
	self assert: rawArguments type isTupleType.
	rawArgumentsLiteralNode := (rawArguments asTupleWithCompileTimeValuesFor: self compilationTarget) asLiteralValueNode
		position: argumentsNode.
		
	convertedArgumentsNode := analyzer addImplicitCastFor: rawArgumentsLiteralNode to: self argumentTupleType.
	arguments := (analyzer evaluateCompileTimeExpression: convertedArgumentsNode) unwrapMbndValue.

	result := self getOrCreateInstanceWithArguments: arguments.
	^ result asLiteralValueNode
		position: node;
		yourself
]

{ #category : #'semantic analysis' }
MbndAbstractTemplate >> analyzeReferenceNode: referenceNode with: analyzer [
	^ self asLiteralValueNode
]

{ #category : #accessing }
MbndAbstractTemplate >> argumentDefinitions [
	^ argumentDefinitions
]

{ #category : #accessing }
MbndAbstractTemplate >> argumentDefinitions: anObject [
	argumentDefinitions := anObject
]

{ #category : #accessing }
MbndAbstractTemplate >> argumentTupleType [
	^ argumentTupleType ifNil: [argumentTupleType := self compilationTarget tuple: (argumentDefinitions collect: [:each | each valueType])]
]

{ #category : #'semantic analysis' }
MbndAbstractTemplate >> asLiteralValueNode [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLiteralValueNode new
		value: self;
		type: self type;
		yourself
]

{ #category : #adding }
MbndAbstractTemplate >> children [
	^ children ifNil: [ children := OrderedCollection new ]
]

{ #category : #accessing }
MbndAbstractTemplate >> compilationEnvironment [
	^ parent compilationEnvironment
]

{ #category : #template }
MbndAbstractTemplate >> getOrCreateInstanceWithArguments: arguments [
	self subclassResponsibility
]

{ #category : #initialization }
MbndAbstractTemplate >> initialize [
	super initialize.
	definitionFragments := OrderedCollection new.
]

{ #category : #testing }
MbndAbstractTemplate >> isTemplate [
	^ true
]

{ #category : #serialization }
MbndAbstractTemplate >> messagePackTypeName [
	^ self class messagePackTypeName
]

{ #category : #accessing }
MbndAbstractTemplate >> module [
	^ parent module
]

{ #category : #accessing }
MbndAbstractTemplate >> name [
	^ name
]

{ #category : #accessing }
MbndAbstractTemplate >> name: anObject [
	name := anObject
]

{ #category : #accessing }
MbndAbstractTemplate >> parent [
	^ parent
]

{ #category : #accessing }
MbndAbstractTemplate >> parent: anObject [
	parent := anObject
]

{ #category : #accessing }
MbndAbstractTemplate >> postMaterializeSetupParent: newParent withModule: newModule [
	self parent: newParent.
	self childrenDo: [ :child |
		child postMaterializeSetupParent: self withModule: newModule
	].
]

{ #category : #accessing }
MbndAbstractTemplate >> type [
	^ self compilationTarget templateType
]
