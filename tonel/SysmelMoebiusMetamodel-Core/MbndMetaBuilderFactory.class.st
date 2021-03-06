Class {
	#name : #MbndMetaBuilderFactory,
	#superclass : #Object,
	#traits : 'TMbndSymbolBinding',
	#classTraits : 'TMbndSymbolBinding classTrait',
	#instVars : [
		'metaBuilderClass'
	],
	#category : #'SysmelMoebiusMetamodel-Core-MetaBuilder'
}

{ #category : #'semantic analysis' }
MbndMetaBuilderFactory >> analyzeReferenceNode: referenceNode ownerProgramEntity: ownerProgramEntity with: analyzer [
	^ MbndAstMetaBuilderInstanceNode new
		instance: (metaBuilderClass new
			ownerProgramEntity: ownerProgramEntity;
			position: referenceNode;
			metaBuilderAnalyzer: analyzer;
			metaBuilderScope: analyzer scope;
			yourself);
		type: analyzer compilationTarget metaBuilderInstanceType;
		position: referenceNode;
		yourself
]

{ #category : #'semantic analysis' }
MbndMetaBuilderFactory >> analyzeReferenceNode: referenceNode with: analyzer [
	^ self analyzeReferenceNode: referenceNode ownerProgramEntity: nil with: analyzer
]

{ #category : #testing }
MbndMetaBuilderFactory >> isMetaBuilderFactory [
	^ true
]

{ #category : #accessing }
MbndMetaBuilderFactory >> metaBuilderClass [
	^ metaBuilderClass
]

{ #category : #accessing }
MbndMetaBuilderFactory >> metaBuilderClass: anObject [
	metaBuilderClass := anObject
]
