Class {
	#name : #MbndCompileTimeConstantMetaBuilder,
	#superclass : #MbndAbstractNamedMetaBuilder,
	#instVars : [
		'valueNode'
	],
	#category : #'SysmelMoebiusMetamodel-Core-MetaBuilder'
}

{ #category : #definition }
MbndCompileTimeConstantMetaBuilder >> assignValue: newValueNode [
	self value: newValueNode
]

{ #category : #'semantic analysis' }
MbndCompileTimeConstantMetaBuilder >> concretizeNodeAt: node withAnalyzer: analyzer [
	name ifNil: [
		analyzer error: 'Alias definition without a name.' at: node
	].

	^ analyzer visitNode: (MbndAstCompileTimeConstantDefinitionNode new
		position: position;
		ownerProgramEntity: ownerProgramEntity;
		name: name;
		value: valueNode;
		yourself)
]

{ #category : #definition }
MbndCompileTimeConstantMetaBuilder >> value: newValueNode [
	valueNode := newValueNode.
]
