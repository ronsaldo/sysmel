Class {
	#name : #Float32x4,
	#superclass : #SysmelPrimitiveVector4Type,
	#category : #'SysmelPharoCompatLayer-Core-PrimitiveTypes'
}

{ #category : #accessing }
Float32x4 class >> elementType [
	^ Float32
]

{ #category : #converting }
Float32x4 >> asColor [
	^ Color r: x encodeSRGBComponent g: y encodeSRGBComponent b: z encodeSRGBComponent alpha: w
]

{ #category : #converting }
Float32x4 >> asFloat32x4 [
	^ self
]
