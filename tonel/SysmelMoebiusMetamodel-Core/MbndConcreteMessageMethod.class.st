Class {
	#name : #MbndConcreteMessageMethod,
	#superclass : #MbndAbstractConcreteMethod,
	#category : #'SysmelMoebiusMetamodel-Core-Method'
}

{ #category : #serialization }
MbndConcreteMessageMethod class >> messagePackTypeName [
	^ #ConcreteMessageMethod
]

{ #category : #visiting }
MbndConcreteMessageMethod >> accept: aVisitor [
	^ aVisitor visitConcreteMessageMethod: self
]

{ #category : #testing }
MbndConcreteMessageMethod >> hasAnonymousName [
	^ name isNil
]

{ #category : #testing }
MbndConcreteMessageMethod >> hasProgramEntityReflectiveNode [
	^ self isDefinition
]

{ #category : #testing }
MbndConcreteMessageMethod >> isMessageMethod [
	^ true
]

{ #category : #'as yet unclassified' }
MbndConcreteMessageMethod >> programEntityInformationDataClass [
	^ self compilationTarget messageMethodInformationClass
]
