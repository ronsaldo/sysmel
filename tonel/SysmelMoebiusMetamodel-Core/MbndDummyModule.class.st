Class {
	#name : #MbndDummyModule,
	#superclass : #MbndAbstractModule,
	#instVars : [
		'compilationTarget'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Module'
}

{ #category : #accessing }
MbndDummyModule >> compilationTarget [
	compilationTarget ifNil: [
		compilationTarget := MbndDummyCompilationTarget basicNew.
		compilationTarget
			coreDefinitionsModule: self;
			initialize
	].

	^ compilationTarget
]