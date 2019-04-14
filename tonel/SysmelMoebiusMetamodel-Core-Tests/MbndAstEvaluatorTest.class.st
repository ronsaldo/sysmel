"
A MbndAstEvaluatorTest is a test class for testing the behavior of MbndAstEvaluator
"
Class {
	#name : #MbndAstEvaluatorTest,
	#superclass : #TestCase,
	#instVars : [
		'module',
		'builder',
		'scope',
		'returnContext',
		'compilationTarget',
		'analyzer',
		'evaluator'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Tests-Evaluation'
}

{ #category : #running }
MbndAstEvaluatorTest >> analyzeAndEvaluateAST: node [
	^ evaluator visitNode: (analyzer visitNode: node)
]

{ #category : #running }
MbndAstEvaluatorTest >> setUp [
	super setUp.
	module := MbndDummyModule new.
	compilationTarget := module compilationTarget.
	builder := MbndAstBuilder forModule: module.

	returnContext := MbndReturnContextScope parent: module programEntityScope.
	scope := MbndLexicalScope parent: returnContext.

	analyzer := MbndAstSemanticAnalyzer new scope: scope; module: module.
	evaluator := MbndAstEvaluator new scope: scope.
]

{ #category : #running }
MbndAstEvaluatorTest >> testEmptySequence [
	| node value |
	node := builder sequence: { }.
	
	value := self analyzeAndEvaluateAST: node.
	self assert: value type isVoidType.
	self assert: value value equals: nil.

]

{ #category : #running }
MbndAstEvaluatorTest >> testLiteral [
	| literal value |
	literal := builder literalInteger: 42.
	value := self analyzeAndEvaluateAST: literal.
	self assert: value unwrapMbndValue equals: 42
]

{ #category : #running }
MbndAstEvaluatorTest >> testLocalDeclaration [
	| node value |
	node := builder sequence: { 
		builder defineLocalVariable: #x withValueType: nil withInitialValue: (builder literalInteger: 5).
		builder identifier: #x
	}.
	
	value := self analyzeAndEvaluateAST: node.
	self assert: value type isIntegerType.
	self assert: value value equals: 5.

]

{ #category : #running }
MbndAstEvaluatorTest >> testNew [
	self assert: evaluator scope equals: scope
]

{ #category : #running }
MbndAstEvaluatorTest >> testReturn [
	| node |
	node := builder return: (builder literalInteger: 42).
	[
		self analyzeAndEvaluateAST: node
	] on: MbndEvaluationReturn do: [ :e |
		self assert: e targetReturnContext equals: returnContext.
		self assert: e returnValue unwrapMbndValue equals: 42
	].
	
]

{ #category : #running }
MbndAstEvaluatorTest >> testSequence [
	| node value |
	node := builder sequence: { 
		builder literalInteger: 1.
		builder literalInteger: 2.
		builder literalInteger: 42.
	}.
	
	value := evaluator visitNode: node.
	self assert: value type isLiteralIntegerType.
	self assert: value value equals: 42.

]