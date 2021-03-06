"
I am a generic language top-level scope.

I introduce the following core control flow top-level macros:

- return:
- if:then:
- if:then:else:
- while:do:
- do:while:

I introduce the following synthesized control-flow macros:
- until:do:
- do:until:

"
Class {
	#name : #MbndGenericLanguageTopLevelScope,
	#superclass : #MbndLanguageTopLevelScope,
	#category : #'SysmelMoebiusMetamodel-Core-Scope'
}

{ #category : #initialization }
MbndGenericLanguageTopLevelScope >> addAllPublicMetaBuilders [
	MbndMetaBuilder withAllSubclassesDo: [ :sc |
		sc globalBindingName ifNotNil: [ :symbol |
			topLevelSymbols at: symbol put: sc metaBuilderFactory
		]
	].
]

{ #category : #initialization }
MbndGenericLanguageTopLevelScope >> addTopLevelMetaBuilders [
	self addAllPublicMetaBuilders
]

{ #category : #initialization }
MbndGenericLanguageTopLevelScope >> addTopLevelSymbols [
	super addTopLevelSymbols.
	topLevelSymbols
		at: #void put: (MbndValue value: nil type: compilationTarget voidType);
		at: #nil put: (MbndValue value: nil type: compilationTarget undefinedType);
		at: #true put: (MbndValue value: true type: compilationTarget literalBooleanTrueType);
		at: #false put: (MbndValue value: false type: compilationTarget literalBooleanFalseType);
		at: module globalNamespace name put: (module globalNamespace asMoebiusValueWithTarget: compilationTarget).
]

{ #category : #'macro expansion' }
MbndGenericLanguageTopLevelScope >> lookupSymbol: aSymbol [
	(super lookupSymbol: aSymbol) ifNotNil: [ :found | ^ found ].
	^ compilationTarget lookupTypeWithName: aSymbol
]
