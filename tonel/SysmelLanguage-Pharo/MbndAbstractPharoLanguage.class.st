Class {
	#name : #MbndAbstractPharoLanguage,
	#superclass : #MbndLanguageInterface,
	#category : #'SysmelLanguage-Pharo'
}

{ #category : #'class initialization' }
MbndAbstractPharoLanguage class >> initialize [
	"This is for ensuring that I am registered."
	MbndLanguageInterface initialize.
]

{ #category : #scope }
MbndAbstractPharoLanguage >> constructTopLevelLanguageScopeFor: aModule [
	^ (MbndSmalltalkLanguageTopLevelScope for: aModule)
		languageName: self languageName;
		yourself
]

{ #category : #'as yet unclassified' }
MbndAbstractPharoLanguage >> convertParseError: message at: index in: sourceCollectionWithContext [
	| parseErrorPosition |
	parseErrorPosition := MbndSourcePosition new
		collection: sourceCollectionWithContext;
		start: index;
		stop: index;
		yourself.
	^ MbndParseError new
		sourcePosition: parseErrorPosition;
		signal: message
]

{ #category : #'as yet unclassified' }
MbndAbstractPharoLanguage >> generateMoebiusASTFromRBAst: rbAST parsedFromSourceCollection: sourceCollection [
	| astBuilder convertedAST |
	astBuilder := MbndAstBuilder forModule: targetModule.
	convertedAST := MbndRBAstConverter new
		strictPharoMode: self strictPharoMode;
		convertAST: rbAST parsedFromSourceCollection: sourceCollection withBuilder: astBuilder.
	self assert: convertedAST isMbndAstNode.
	^ convertedAST
]

{ #category : #'as yet unclassified' }
MbndAbstractPharoLanguage >> strictPharoMode [
	^ true
]
