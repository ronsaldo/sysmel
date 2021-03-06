Class {
	#name : #MbndAbstractModule,
	#superclass : #Object,
	#traits : 'TMbndModule',
	#classTraits : 'TMbndModule classTrait',
	#instVars : [
		'name',
		'namedTargetCodeModules'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Module'
}

{ #category : #serialization }
MbndAbstractModule class >> messagePackTypeName [
	^ nil
]

{ #category : #evaluation }
MbndAbstractModule >> analyzeAndEvaluateAST: ast [
	| parsedSource |
	parsedSource := MbndLanguageParsedSource new
		ast: ast;
		language: MbndGenericMacroLanguageInterface new.
	^ self analyzeAndEvaluateParsedSource: parsedSource
]

{ #category : #evaluation }
MbndAbstractModule >> analyzeAndEvaluateAST: ast inScope: scope [
	| analyzerAndEvaluator |
	analyzerAndEvaluator := MbndAstSemanticAnalyzerAndEvaluator new
		module: self;
		scope: scope.
	^ MbndProfiler component: #analysisAndEvaluation with: [analyzerAndEvaluator analyzeAndEvaluateFunctionContent: ast]

]

{ #category : #'as yet unclassified' }
MbndAbstractModule >> generateTargetCodeModule [
	^ self getTargetCodeModuleNamed: #main orCreateWith: [
		| codeGenerator |
		codeGenerator := self compilationTarget newCodeModuleGenerator.
	 	MbndProfiler component: #irCodeGeneration with: [codeGenerator generateCodeModuleFor: self name: #main]
	]
]

{ #category : #'as yet unclassified' }
MbndAbstractModule >> getTargetCodeModuleNamed: codeModuleName orCreateWith: aBlock [
	^ self namedTargetCodeModules at: codeModuleName ifAbsent: aBlock
]

{ #category : #accessing }
MbndAbstractModule >> importedModules [
	^ #()
]

{ #category : #serialization }
MbndAbstractModule >> messagePackTypeName [
	^ self class messagePackTypeName
]

{ #category : #accessing }
MbndAbstractModule >> name [
	^ name ifNil: [ name := #__AnonModule__ ]
]

{ #category : #accessing }
MbndAbstractModule >> name: anObject [
	name := anObject
]

{ #category : #accessing }
MbndAbstractModule >> namedTargetCodeModules [
	^ namedTargetCodeModules ifNil: [ namedTargetCodeModules := Dictionary new ]
]

{ #category : #printing }
MbndAbstractModule >> printOn: aStream [
	aStream nextPutAll: 'module '; nextPutAll: name asString
]
