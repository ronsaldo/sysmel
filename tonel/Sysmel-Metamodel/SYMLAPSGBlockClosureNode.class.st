Class {
	#name : #SYMLAPSGBlockClosureNode,
	#superclass : #SYMLAPSGNode,
	#instVars : [
		'body',
		'arguments',
		'returnTypeExpression',
		'returnType'
	],
	#category : #'Sysmel-Metamodel-APSG'
}

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> arguments [
	<compileTime>
	^ arguments
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> arguments: anObject [
	<compileTime>
	arguments := anObject
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> body [
	<compileTime>
	^ body
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> body: anObject [
	<compileTime>
	body := anObject
]

{ #category : #testing }
SYMLAPSGBlockClosureNode >> isBlockClosure [
	<compileTime>
	^ true
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> returnType [
	^ returnType
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> returnType: anObject [
	returnType := anObject
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> returnTypeExpression [
	<compileTime>
	^ returnTypeExpression
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> returnTypeExpression: anObject [
	<compileTime>
	returnTypeExpression := anObject
]

{ #category : #accessing }
SYMLAPSGBlockClosureNode >> semanticAnalysisInEnvironment: environment [
	| closureEnvironment analyzedArguments analyzedReturnType blockClosure analyzedBody returnTypeCoercionRule |
	blockClosure := SYMLBlockClosure new
		parent: environment definitionProgramEntity;
		module: environment definitionProgramEntity module;
		definitionNode: self;
		definitionEnvironment: environment;
		yourself.
	closureEnvironment := (SYMLBlockClosureAnalysisScope parent: environment)
		blockClosure: blockClosure.

	"Analyze the arguments."
	analyzedArguments := arguments collect: [ :arg | arg semanticAnalysisInEnvironment: closureEnvironment ].

	"Analyze the body."
	analyzedBody := body semanticAnalysisInEnvironment: closureEnvironment.

	"Analyze the return type"
	analyzedReturnType := returnType.
	analyzedReturnType ifNil: [ 
		returnTypeExpression ifNotNil: [ 
			analyzedReturnType := (returnTypeExpression analyzeAndEvaluateInEnvironment: environment) unwrapSysmelValue.
		] ifNil: [ 
			"Infer the return type from the analyzed block body."
			analyzedReturnType := (analyzedBody evaluateTypeInEnvironment: environment) valueType 
		].
	
		analyzedReturnType ifNil: [ 
			self error: 'A return type specification is required.' at: position
		]
	].

	returnTypeCoercionRule := analyzedReturnType isVoidType
		ifTrue: [ nil ]
		ifFalse: [ (analyzedBody evaluateTypeInEnvironment: closureEnvironment)
			node: body coercedImplicitlyInto: analyzedReturnType at: self ].
	
	blockClosure
		arguments: analyzedArguments;
		returnType: analyzedReturnType;
		body: analyzedBody;
		returnTypeCoercionRule: returnTypeCoercionRule.
	
	^ SYMLAPSGAnalyzedBlockClosureNode new
		position: position;
		blockClosure: blockClosure;
		yourself
]