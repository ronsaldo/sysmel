Class {
	#name : #UInt32,
	#superclass : #SysmelPrimitiveUnsignedIntegerType,
	#category : #'SysmelPharoCompatLayer-Core-PrimitiveTypes'
}

{ #category : #sizes }
UInt32 class >> instanceAlignment [
	^ 4
]

{ #category : #sizes }
UInt32 class >> instanceSize [
	^ 4
]