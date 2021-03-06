Class {
	#name : #MbndAstBuilder,
	#superclass : #Object,
	#instVars : [
		'module',
		'gensymContext',
		'defaultPosition'
	],
	#category : #'SysmelMoebiusMetamodel-Core-AST'
}

{ #category : #'as yet unclassified' }
MbndAstBuilder class >> forModule: aModule [
	^ self new module: aModule; yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> alias: name definition: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstCompileTimeConstantDefinitionNode new
		position: defaultPosition;
		name: name;
		value: value;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> argument: name type: valueType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstArgumentDefinitionNode new
		position: defaultPosition;
		name: name;
		valueType: valueType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> array: array elementReferenceAt: index [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstArrayElementReferenceNode new
		position: defaultPosition;
		container: array;
		index: index;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> arraySlice: array elementReferenceAt: index [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstArraySliceElementReferenceNode new
		position: defaultPosition;
		container: array;
		index: index;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> associationKey: key value: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstAssociationNode new
		position: defaultPosition;
		key: key;
		value: value;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> bitCast: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstBitCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> blockClosure: body arguments: arguments returnType: returnType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstBlockClosureNode new
		position: defaultPosition;
		arguments: arguments;
		returnType: returnType;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> blockClosureWithArguments: arguments returnType: returnType body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstBlockClosureNode new
		position: defaultPosition;
		arguments: arguments;
		returnType: returnType;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> breakStatement [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstBreakStatementNode new
		position: defaultPosition;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> call: function arguments: arguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFunctionCallNode new
		position: defaultPosition;
		function: function;
		arguments: arguments;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> casesOf: matchedExpression cases: cases [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstCasesOfNode new
		position: defaultPosition;
		matchedExpression: matchedExpression;
		cases: cases;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> casesOfCase: pattern do: action [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstCasesOfCaseNode new
		position: defaultPosition;
		pattern: pattern;
		action: action;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> cleanUpScope: expression actions: cleanUpActions [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self exceptionScope: expression catchExpressions: #() cleanUpActions: cleanUpActions
]

{ #category : #accessing }
MbndAstBuilder >> compilationEnvironment [
	^ module compilationEnvironment
]

{ #category : #accessing }
MbndAstBuilder >> compilationTarget [
	^ module compilationTarget
]

{ #category : #'builder methods' }
MbndAstBuilder >> constructAggregate: aggregate withNamedElements: elements [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstAggregateConstructionWithNamedElementsNode new
		position: defaultPosition;
		aggregate: aggregate;
		elements: elements;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> constructAggregate: aggregate withOrderedElements: elements [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstAggregateConstructionWithOrderedElementsNode new
		position: defaultPosition;
		aggregate: aggregate;
		elements: elements;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> constructArrayOf: elementType withElements: elements [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstArrayConstructionNode new
		position: defaultPosition;
		elementType: elementType;
		elements: elements;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> constructMatrix: matrixType withColumns: columns [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixConstructionNode new
		position: defaultPosition;
		matrixType: matrixType;
		columns: columns;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> constructVector: vectorType withElements: elements [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVectorConstructionNode new
		position: defaultPosition;
		vectorType: vectorType;
		elements: elements;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> continueStatement [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstContinueStatementNode new
		position: defaultPosition;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareClass: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstClassDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareEnum: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstEnumDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareGCClass: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstGCClassDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareInterface: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstInterfaceDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareStructure: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstStructureDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareTrait: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTraitDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> declareUnion: name [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstUnionDeclarationNode new
		position: defaultPosition;
		name: name;
		yourself
]

{ #category : #accessing }
MbndAstBuilder >> defaultPosition [
	^ defaultPosition
]

{ #category : #accessing }
MbndAstBuilder >> defaultPosition: anObject [
	defaultPosition := anObject
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineClass: declaration superclass: superclass body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstClassDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		superclass: superclass;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineClassNamed: name superclass: superclass body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineClass: (self declareClass: name) superclass: superclass body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineEnum: declaration valueType: valueType body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineEnum: declaration valueType: valueType values: nil body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineEnum: declaration valueType: valueType values: values body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstEnumDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		valueType: valueType;
		values: values;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineEnumNamed: name valueType: valueType body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineEnum: (self declareEnum: name) valueType: valueType values: nil body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineEnumNamed: name valueType: valueType values: values body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineEnum: (self declareEnum: name) valueType: valueType values: values body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineField: name withValueType: valueType withInitialValue: initialValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFieldDefinitionNode new
		position: defaultPosition;
		name: name;
		valueType: valueType;
		initialValue: initialValue;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineGCClass: declaration superclass: superclass body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstGCClassDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		superclass: superclass;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineGCClassNamed: name superclass: superclass body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineGCClass: (self declareGCClass: name) superclass: superclass body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineGlobalVariable: name withValueType: valueType withInitialValue: initialValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstGlobalVariableDefinitionNode new
		position: defaultPosition;
		name: name;
		valueType: valueType;
		initialValue: initialValue;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineInterface: declaration body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstInterfaceDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineInterfaceNamed: name body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineInterface: (self declareInterface: name) body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineLocalVariable: name withValueType: valueType withInitialValue: initialValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLocalVariableDefinitionNode new
		position: defaultPosition;
		name: name;
		valueType: valueType;
		initialValue: initialValue;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> definePackedStructureNamed: name body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineStructure: (self declareStructure: name) packed body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineStructure: declaration body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstStructureDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineStructureNamed: name body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineStructure: (self declareStructure: name) body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineTemplate: name arguments: arguments body: body [
	<compileTime>
	^ MbndAstTemplateDefinitionNode new
		position: defaultPosition;
		name: name;
		arguments: arguments;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineTrait: declaration body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTraitDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineTraitNamed: name body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineTrait: (self declareTrait: name) body: body
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineUnion: declaration body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstUnionDefinitionNode new
		position: defaultPosition;
		declaration: declaration;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> defineUnionNamed: name body: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self defineUnion: (self declareUnion: name) body: body
]

{ #category : #'as yet unclassified' }
MbndAstBuilder >> delegateToMetaBuilderClass: metaBuilderClass withAnalyzer: analyzer at: position [
	^ MbndAstMetaBuilderInstanceNode new
		instance: (metaBuilderClass new
			position: position;
			metaBuilderAnalyzer: analyzer;
			metaBuilderScope: analyzer scope;
			yourself);
		type: analyzer compilationTarget metaBuilderInstanceType;
		position: position;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> dictionary: aSequenceOfNodes [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstDictionaryNode new
		position: defaultPosition;
		elements: aSequenceOfNodes;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> discardFragment [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstDiscardFragmentStatementNode new
		position: defaultPosition;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> do: action continueWith: continueAction while: condition [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstDoWhileLoopNode new
		position: defaultPosition;
		action: action;
		continueAction: continueAction;
		condition: condition lexicallyScoped;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> do: action while: condition [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self do: action continueWith: nil while: condition
]

{ #category : #'builder methods' }
MbndAstBuilder >> exceptionCatch: exceptionTypeExpression doing: catchAction [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExceptionCatchExpressionNode new
		position: defaultPosition;
		exceptionTypeExpression: exceptionTypeExpression;
		catchAction: catchAction;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> exceptionLexicalScope: expression catchExpressions: catchExpressions cleanUpActions: cleanUpActions [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLexicalScopeNode new
		position: defaultPosition;
		expression: expression;
		catchExpressions: catchExpressions;
		explicitCleanUpActions: cleanUpActions;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> exceptionScope: expression catchExpressions: catchExpressions cleanUpActions: cleanUpActions [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExceptionScopeNode new
		position: defaultPosition;
		expression: expression;
		catchExpressions: catchExpressions;
		explicitCleanUpActions: cleanUpActions;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> expandScalar: scalar toMatrixType: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExpandScalarToMatrixCastNode new
		position: defaultPosition;
		expression: scalar;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> expandScalar: scalar toVectorType: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExpandScalarToVectorCastNode new
		position: defaultPosition;
		expression: scalar;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> explicitCast: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExplicitCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'as yet unclassified' }
MbndAstBuilder >> extractVariantTypeSelector: variant [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVariantTypeSelectorExtractionNode new
		position: defaultPosition;
		variant: variant;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> floatingPoint: expression toInteger: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFloatingPointToIntegerCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> floatingPointSize: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFloatingPointSizeCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> functionNamed: name arguments: arguments returnType: returnType definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self functionPrototype: (self functionPrototypeNamed: name arguments: arguments returnType: returnType) definitionBody: definitionBody
]

{ #category : #'builder methods' }
MbndAstBuilder >> functionPrototype: prototype definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFunctionDefinitionNode new
		position: defaultPosition;
		prototype: prototype;
		body: definitionBody;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> functionPrototypeNamed: name arguments: arguments returnType: returnType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstFunctionPrototypeNode new
		position: defaultPosition;
		name: name;
		arguments: arguments;
		returnType: returnType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> gensym: symbolName [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self identifier: (MbndGeneratedSymbol new 
		name: symbolName;
		context: gensymContext;
		yourself)
]

{ #category : #accessing }
MbndAstBuilder >> gensymContext [
	^ gensymContext
]

{ #category : #accessing }
MbndAstBuilder >> gensymContext: anObject [
	gensymContext := anObject
]

{ #category : #'builder methods' }
MbndAstBuilder >> getAggregate: aggregate referenceOfField: field [
	<compileTime>
	^ MbndAstGetFieldReferenceNode new
		position: defaultPosition;
		aggregate: aggregate;
		field: field;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> getAggregate: aggregate valueOfField: field [
	<compileTime>
	^ MbndAstGetFieldValueNode new
		position: defaultPosition;
		aggregate: aggregate;
		field: field;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> getGCVariableDataArraySize: container [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstGetGCVariableDataArraySizeNode new
		position: defaultPosition;
		container: container;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> identifier: symbol [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstIdentifierReferenceNode new
		position: defaultPosition;
		symbol: symbol;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> if: condition trueValue: trueValue falseValue: falseValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstIfConditionNode new
		position: defaultPosition;
		condition: condition;
		trueValue: trueValue;
		falseValue: falseValue;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> implicitCast: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstImplicitCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #initialization }
MbndAstBuilder >> initialize [
	super initialize.
	defaultPosition := MbndSourcePosition empty.
]

{ #category : #'builder methods' }
MbndAstBuilder >> inlineAssembly: assembly architecture: architecture dialect: dialect inputList: inputList outputList: outputList clobberList: clobberList [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstInlineAssembly new
		position: defaultPosition;
		assembly: assembly;
		architecture: architecture;
		dialect: dialect;
		inputList: inputList;
		outputList: outputList;
		clobberList: clobberList;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> integer: expression toFloatingPoint: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstIntegerToFloatingPointCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> lexicalScope: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLexicalScopeNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> literal: literalValue type: type [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLiteralValueNode new
		position: defaultPosition;
		value: literalValue;
		type: type;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> literal: literalValue typeName: typeSelector [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstLiteralValueNode new
		position: defaultPosition;
		value: literalValue;
		type: (self lookupTypeWithName: typeSelector);
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalArray: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralArray
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalBoolean: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ literalValue
		ifTrue: [ self literalBooleanTrue ]
		ifFalse: [ self literalBooleanFalse ]
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalBooleanFalse [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: false typeName: #_LiteralBooleanFalse
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalBooleanTrue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: true typeName: #_LiteralBooleanTrue
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalCharacter: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralCharacter
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalFloat: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralFloat
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalInteger: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralInteger
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalString: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralString
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalString: string position: position [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ (self literalString: string)
		position: position
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalSymbol: literalValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: literalValue typeName: #_LiteralSymbol
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalUndefined [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: nil typeName: MbndUndefinedType typeName
]

{ #category : #'builder methods' }
MbndAstBuilder >> literalVoid [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self literal: nil typeName: MbndVoidType typeName
]

{ #category : #'builder methods' }
MbndAstBuilder >> lookupTypeWithName: typeSelector [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self compilationTarget lookupTypeWithName: typeSelector
]

{ #category : #'builder methods' }
MbndAstBuilder >> macroInlineBlock: blockValue [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ blockValue isBlockClosureNode ifTrue: [
		MbndAstImmediateBlockClosureActivationNode new
			position: blockValue;
			blockNode: blockValue;
			arguments: #();
			yourself
	] ifFalse: [
		MbndAstBlockClosureActivationAttemptNode new
			position: blockValue;
			blockNode: blockValue;
			arguments: #();
			yourself.
	]
]

{ #category : #'builder methods' }
MbndAstBuilder >> macroMessagePrototype: prototype definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMacroMessageMethodDefinitionNode new
		position: defaultPosition;
		prototype: prototype;
		body: definitionBody;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> macroMessagePrototypeSelector: selector arguments: arguments returnType: returnType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMacroMessageMethodPrototypeNode new
		position: defaultPosition;
		selector: selector;
		arguments: arguments;
		returnType: returnType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> macroMessageSelector: selector arguments: arguments returnType: returnType definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self macroMessagePrototype: (self macroMessagePrototypeSelector: selector arguments: arguments returnType: returnType)
		definitionBody: definitionBody
]

{ #category : #'builder methods' }
MbndAstBuilder >> matrix: matrix getColumn: column [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixGetColumnNode new
		position: defaultPosition;
		matrix: matrix;
		element: column;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> matrix: matrix getRow: row [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixGetRowNode new
		position: defaultPosition;
		matrix: matrix;
		element: row;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> matrix: leftMatrix multiplicationWith: rightMatrix [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixMultiplicationOperationNode new
		position: defaultPosition;
		leftMatrix: leftMatrix;
		rightMatrix: rightMatrix;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> matrix: matrix setColumn: column value: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixSetColumnValueNode new
		position: defaultPosition;
		matrix: matrix;
		element: column;
		value: value;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> matrix: matrix setRow: row value: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixSetRowValueNode new
		position: defaultPosition;
		matrix: matrix;
		element: row;
		value: value;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> messageChainMessageSelector: selector arguments: arguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMessageChainMessageNode new
		position: defaultPosition;
		selector: selector;
		arguments: arguments;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> messageChainReceiver: receiver messages: chainedMessages [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMessageChainNode new
		position: defaultPosition;
		receiver: receiver;
		messages: chainedMessages;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> messagePrototype: prototype definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMessageMethodDefinitionNode new
		position: defaultPosition;
		prototype: prototype;
		body: definitionBody;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> messagePrototypeSelector: selector arguments: arguments returnType: returnType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMessageMethodPrototypeNode new
		position: defaultPosition;
		selector: selector;
		arguments: arguments;
		returnType: returnType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> messageSelector: selector arguments: arguments returnType: returnType definitionBody: definitionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self messagePrototype: (self messagePrototypeSelector: selector arguments: arguments returnType: returnType)
		definitionBody: definitionBody
]

{ #category : #accessing }
MbndAstBuilder >> module [
	^ module
]

{ #category : #accessing }
MbndAstBuilder >> module: aModule [
	module := aModule
]

{ #category : #'builder methods' }
MbndAstBuilder >> namespace: name definition: body [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstNamespaceDefinitionNode new
		position: defaultPosition;
		name: name;
		body: body;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> pointer: pointer elementReferenceAt: index [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstPointerElementReferenceNode new
		position: defaultPosition;
		container: pointer;
		index: index;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> pointer: pointer toArraySliceFrom: firstIndex until: lastIndex [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstPointerToArraySliceNode new
		position: defaultPosition;
		pointer: pointer;
		firstIndex: firstIndex;
		lastIndex: lastIndex;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> pointerAsReference: pointer [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstPointerAsReferenceNode new
		position: defaultPosition;
		pointer: pointer;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> pragmaSelector: selector arguments: arguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstPragmaNode new
		position: defaultPosition;
		selector: selector;
		arguments: arguments;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> pragmas: aSequenceOfPragmas sequence: aSequenceOfNodes [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSequenceNode new
		position: defaultPosition;
		pragmas: aSequenceOfPragmas;
		elements: aSequenceOfNodes;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> programEntity: programEntity extension: extensionBody [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstProgramEntityExtensionNode new
		position: defaultPosition;
		programEntity: programEntity;
		body: extensionBody;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> quasiQuote: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstQuasiQuoteNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> quasiUnquote: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstQuasiUnquoteNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> quote: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstQuoteNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> receiveFromInputChannel: channel [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstIOChannelReceiveAction new
		position: defaultPosition;
		channel: channel;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> reference: reference assignValue: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstAssignmentNode new
		position: defaultPosition;
		reference: reference;
		value: value;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> referenceAsPointer: reference [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstReferenceAsPointerNode new
		position: defaultPosition;
		reference: reference;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> referenceLoad: reference [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstReferenceLoadNode new
		position: defaultPosition;
		reference: reference;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> reinterpretCast: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstReinterpretCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> return: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstReturnStatementNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> sampleWith: sampler texture: texture at: texcoord depth: depthReference [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSampleTextureWithDepthReferenceNode new
		position: defaultPosition;
		sampler: sampler;
		texture: texture;
		texcoord: texcoord;
		depthReference: depthReference;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> sampleWith: sampler texture: texture at: texcoord dtdx: dtdx dtdy: dtdy [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSampleTextureWithExplicitDerivativesNode new
		position: defaultPosition;
		sampler: sampler;
		texture: texture;
		texcoord: texcoord;
		dtdx: dtdx;
		dtdy: dtdy;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> sampleWith: sampler texture: texture at: texcoord lod: lod [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSampleTextureWithExplicitLodNode new
		position: defaultPosition;
		sampler: sampler;
		texture: texture;
		texcoord: texcoord;
		lod: lod;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> sampleWith: sampler texture: texture atWithImplicitLod: texcoord [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSampleTextureWithImplicitLodNode new
		position: defaultPosition;
		sampler: sampler;
		texture: texture;
		texcoord: texcoord;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> send: selector to: aReceiver withArguments: arguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstExpandedMessageSendNode new
		position: defaultPosition;
		selector: selector;
		receiver: aReceiver;
		arguments: arguments;
		yourself
		
]

{ #category : #'builder methods' }
MbndAstBuilder >> sendIntoOutputChannel: channel message: message [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstIOChannelSendAction new
		position: defaultPosition;
		channel: channel;
		message: message;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> sequence: aSequenceOfNodes [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self pragmas: #() sequence: aSequenceOfNodes
]

{ #category : #'builder methods' }
MbndAstBuilder >> setAggregate: aggregate field: field value: newValue [
	<compileTime>
	^ MbndAstSetFieldValueNode new
		position: defaultPosition;
		aggregate: aggregate;
		field: field;
		newValue: newValue;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> signExtend: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSignExtensionCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> splice: expression [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstSpliceNode new
		position: defaultPosition;
		expression: expression;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> temporaryVariableOfType: valueType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTemporaryVariableNode new
		position: defaultPosition;
		valueType: valueType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> thisContext [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstThisContextReferenceNode new
		position: defaultPosition;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> transposeMatrix: matrix [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstMatrixTransposeNode new
		position: defaultPosition;
		matrix: matrix;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> truncate: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTruncateCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> tuple: aSequenceOfNodes [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTupleNode new
		position: defaultPosition;
		elements: aSequenceOfNodes;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> tuple: tuple elementReferenceAt: index [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstTupleElementReferenceNode new
		position: defaultPosition;
		container: tuple;
		index: index;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> unexpandedCall: calledFunction withArguments: aCollectionOfArguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self unexpandedSend: (self literalSymbol: #applyWithArguments:) to: calledFunction withArguments: { 		self tuple: aCollectionOfArguments
	}
]

{ #category : #'builder methods' }
MbndAstBuilder >> unexpandedSend: selector to: aReceiver withArguments: arguments [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstUnexpandedMessageSendNode new
		position: defaultPosition;
		selector: selector;
		receiver: aReceiver;
		arguments: arguments;
		yourself
		
]

{ #category : #'builder methods' }
MbndAstBuilder >> unwrapEnumValue: node [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstUnwrapEnumValueCastNode new
		position: defaultPosition;
		expression: node;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> valistEnd: valist [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVAListEndExpressionNode new
		position: defaultPosition;
		valist: valist;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> valistStart: valist [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVAListStartExpressionNode new
		position: defaultPosition;
		valist: valist;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> variableReference: binding [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVariableReference new
		position: defaultPosition;
		binding: binding;
		yourself
]

{ #category : #'as yet unclassified' }
MbndAstBuilder >> variant: variant getFieldOfType: type [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVariantUnboxingNode new
		position: defaultPosition;
		variant: variant;
		typeSelector: type;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> vector: vector getElement: element [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self vector: vector getElements: { element }
]

{ #category : #'builder methods' }
MbndAstBuilder >> vector: vector getElements: elements [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVectorGetElementsNode new
		position: defaultPosition;
		vector: vector;
		elements: elements;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> vector: vector setElement: element value: value [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self vector: vector setElements: { element } values: { value }
]

{ #category : #'builder methods' }
MbndAstBuilder >> vector: vector setElements: elements values: values [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstVectorSetElementsNode new
		position: defaultPosition;
		vector: vector;
		elements: elements;
		values: values
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> while: condition do: action [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ self while: condition do: action continueWith: nil
]

{ #category : #'builder methods' }
MbndAstBuilder >> while: condition do: action continueWith: continueAction [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstWhileLoopNode new
		position: defaultPosition;
		condition: condition lexicallyScoped;
		action: action;
		continueAction: continueAction;
		yourself
]

{ #category : #'builder methods' }
MbndAstBuilder >> zeroExtend: expression to: targetType [
	<compileTime>
	<returnType: #_ASTNodeType>
	^ MbndAstZeroExtensionCastNode new
		position: defaultPosition;
		expression: expression;
		targetType: targetType;
		yourself
]
