Class {
	#name : #MbndIntegerToPointerTypeConversionRule,
	#superclass : #MbndSpecificCastNodeRewriteTypeConversionRule,
	#category : #'SysmelMoebiusMetamodel-Core-TypeConversion'
}

{ #category : #hooks }
MbndIntegerToPointerTypeConversionRule >> specificCastNodeClass [
	^ MbndAstIntegerToPointerCastNode
]
