Class {
	#name : #MbndAstSemanticAnalyzerAndEvaluator,
	#superclass : #MbndAstDelegationVisitor,
	#instVars : [
		'evaluator',
		'semanticAnalyzer'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Evaluation'
}

{ #category : #'as yet unclassified' }
MbndAstSemanticAnalyzerAndEvaluator >> analyzeAndEvaluateFunctionContent: ast [
	| returnContext |
	returnContext := evaluator scope returnContextScope.
	
	[ 
		^ self visitNode: ast
	] on: MbndEvaluationReturn do: [ :e |
		e targetReturnContext == returnContext
			ifTrue: [ ^ e returnValue ]
			ifFalse: [ ^ e pass ]
	]
	
]

{ #category : #delegate }
MbndAstSemanticAnalyzerAndEvaluator >> analyzeAndEvaluateNode: aNode [
	^ self evaluateNode: (self analyzeNode: aNode)
]

{ #category : #delegate }
MbndAstSemanticAnalyzerAndEvaluator >> analyzeNode: aNode [
	^ semanticAnalyzer analyzeNodeIfNeeded: aNode
]

{ #category : #delegate }
MbndAstSemanticAnalyzerAndEvaluator >> delegateNodeVisit: aNode [
	^ self analyzeAndEvaluateNode: aNode
]

{ #category : #operations }
MbndAstSemanticAnalyzerAndEvaluator >> evaluateNode: aNode [
	^ evaluator visitNode: aNode
]

{ #category : #'as yet unclassified' }
MbndAstSemanticAnalyzerAndEvaluator >> evaluateNode: node withReturnContextInScope: parentScope [
	| returnContext |
	returnContext := parentScope newReturnContextScope.
	self scope: (returnContext newLexicalScopeAt: node).
	
	[
		^ self visitNode: node
	] on: MbndEvaluationReturn do: [ :e |
		e targetReturnContext == returnContext
			ifTrue: [ ^ e returnValue ]
			ifFalse: [ ^ e pass ]
	]
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> evaluator [
	^ evaluator ifNil: [ evaluator := MbndAstEvaluator new ]
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> evaluator: anObject [
	evaluator := anObject
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> module: aModule [
	self semanticAnalyzer module: aModule.
	self evaluator module: aModule.
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> scope: aScope [
	self semanticAnalyzer scope: aScope.
	self evaluator scope: aScope.
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> semanticAnalyzer [
	^ semanticAnalyzer ifNil: [ semanticAnalyzer := MbndAstSemanticAnalyzer new ]
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> semanticAnalyzer: anObject [
	semanticAnalyzer := anObject
]

{ #category : #delegate }
MbndAstSemanticAnalyzerAndEvaluator >> visitLexicalScopeNode: node [
	| newScope |
	newScope := evaluator scope newLexicalScopeAt: node.
	^ self withScope: newScope do: [
		self visitNode: node expression
	]
]

{ #category : #delegate }
MbndAstSemanticAnalyzerAndEvaluator >> visitSequenceNode: node [
	| result |
	self visitNodes: node pragmas.
	node elements ifEmpty: [
		^ MbndValue value: nil type: node type
	].

	result := nil.
	node elements do: [ :element |
		result := self visitNode: element
	].

	^ result
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> withAnalysisScope: newAnalysisScope evaluationScope: newEvaluationScope do: aBlock [
	| oldAnalysisScope oldEvaluationScope |
	oldAnalysisScope := semanticAnalyzer scope.
	oldEvaluationScope := evaluator scope.

	semanticAnalyzer scope: newAnalysisScope.
	evaluator scope: newEvaluationScope.
	
	^ aBlock ensure: [ 
		semanticAnalyzer scope: oldAnalysisScope.
		evaluator scope: oldEvaluationScope
	]
]

{ #category : #accessing }
MbndAstSemanticAnalyzerAndEvaluator >> withScope: newScope do: aBlock [
	^ self withAnalysisScope: newScope evaluationScope: newScope do: aBlock
]
