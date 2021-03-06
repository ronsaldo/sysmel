Class {
	#name : #MbndImplicitUpcastTypeConversionRule,
	#superclass : #MbndAstRewriteTypeConversionRule,
	#category : #'SysmelMoebiusMetamodel-Core-TypeConversion'
}

{ #category : #hooks }
MbndImplicitUpcastTypeConversionRule >> applySpecificIdentityCastToNode: node withAnalyzer: analyzer [
	self flag: 'TODO: Some of these cast may require special nodes.'.
	^ node expression
]

{ #category : #testing }
MbndImplicitUpcastTypeConversionRule >> canBeUsedToConvertNode: node from: sourceType into: targetType [
	(sourceType isAbstractClassType and: [ targetType isAbstractClassType ]) ifTrue: [ ^ false ].
	^ true
]

{ #category : #acccessing }
MbndImplicitUpcastTypeConversionRule >> conversionCost [
	^ 1
]
