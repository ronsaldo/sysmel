Class {
	#name : #MbndAbstractModuleVisitor,
	#superclass : #Object,
	#traits : 'TMbndObject',
	#classTraits : 'TMbndObject classTrait',
	#category : #'SysmelMoebiusMetamodel-Core-Module'
}

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateBasicInitializeMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateCopyAssignmentMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateFinalizeMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateInitializeFromCopyMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateInitializeMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateInitializeMovingFromMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAggregateMoveAssignmentMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitArraySliceType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitArrayType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitAssociationType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitBlockClosureMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitClassType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitCompileTimeConstant: compileTimeConstant [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitCompileTimeVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitCompilerObjectType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitConcreteFunclet: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitConcreteFunctionMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitConcreteMessageMethod: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitConstructorGroup: functionGroup [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitDecoratedType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitEnumType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFieldVariable: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionArgumentVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionCapturedVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionGroup: functionGroup [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionGroupType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionLocalVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionReceiverArgumentVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitFunctionType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitGCBlockClosureType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitGCClassType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitGlobalVariable: variable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitInputChannelType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitIntrinsicArithmeticBinaryOperationMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitIntrinsicArithmeticComparisonMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitIntrinsicArithmeticUnaryOperationMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitIntrinsicPointerComparisonMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitIntrinsicThreeWayComparisonMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitMacroMessageMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitMessageGroup: functionGroup [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitModule: module [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitNamespace: namespace [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitNamespaceType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitNativeBlockClosureType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitOutputChannelType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPointerType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPredicatedTypeConcept: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveBooleanType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveCharacterCodeType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveFloatType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveIntegerType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveMatrixType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitPrimitiveVectorType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitReferenceType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitReturnType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitSamplerType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitSimpleMetaType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitStructureType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitSystemProvidedTypeTemplate: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTargetNamedIntrinsicMethod: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTemplate: template [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTemplateInstance: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTemporaryReferenceType: method [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTextureType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitTupleType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitUndefinedType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitUnionType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitVAListType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitVTablePointerFieldVariable: vtableFieldVariable [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitVariantUnionType: type [
	self subclassResponsibility
]

{ #category : #visiting }
MbndAbstractModuleVisitor >> visitVoidType: type [
	self subclassResponsibility
]
