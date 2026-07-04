# Blueprint Variables

Variables are defined with `<Variable>` elements inside `<Blueprint>` or `<WidgetBlueprint>`.

## `<Blueprint>` and `<WidgetBlueprint>` attributes

| Attribute | Example | Description |
|---|---|---|
| `Super` | `Super="Actor"` | Parent class. Default: `UObject` (or `UWidgetMarkupUserWidget` for WidgetBlueprint) |
| `Implements` | `Implements="MyInterface,OtherInterface"` | Comma-separated interface names. Each must be a `UInterface` subclass. |
| `Script` | `Script="Samples.MyComponent"` | Python component module path (WidgetBlueprint only) |

```xml
<Blueprint Super="Actor" Implements="AbilitySystemInterface">
  <Variable Name="Health" Type="Float" Default="100.0" />
</Blueprint>

<WidgetBlueprint Super="WidgetMarkupUserWidget" Script="Samples.MyComponent">
  <WidgetTree>...</WidgetTree>
</WidgetBlueprint>
```

> **WidgetBlueprint Super constraint:** Must be a subclass of `UWidgetMarkupUserWidget`.

## Basic types

Type names are **PascalCase and case-sensitive**. These match UE's blueprint display names.

| Type | Display Name | C++ Type | Default example |
|---|---|---|---|
| `Boolean` | Boolean | `bool` | `"True"` |
| `Integer` | Integer | `int32` | `"0"` |
| `Integer64` | Integer64 | `int64` | `"0"` |
| `Float` | Float | `float` | `"1.5"` |
| `Double` | Double | `double` | `"3.14"` |
| `Byte` | Byte | `uint8` | `"255"` |
| `String` | String | `FString` | `"hello"` |
| `Text` | Text | `FText` | `"hello"` |
| `Name` | Name | `FName` | `"MyName"` |

```xml
<Variable Name="Health" Type="Float" Default="100.0" />
<Variable Name="IsAlive" Type="Boolean" Default="True" />
```

## Struct types

Struct names omit the `F` prefix. Case-sensitive exact match. `Vector` → `FVector`.

> **Ref:** `FTypeParser::ResolveStruct()` — tries `TryFindTypeSlow<UScriptStruct>` exact match first, then `"F"+Token`, then fallback iteration.

| Type | UE Struct | Default example | Description |
|---|---|---|---|
| `Vector` | FVector | `"0,0,0"` | X, Y, Z |
| `Vector2D` | FVector2D | `"0,0"` | X, Y |
| `Rotator` | FRotator | `"0,0,0"` | Pitch, Yaw, Roll |
| `Transform` | FTransform | `"0,0,0,0,0,0,1,1,1"` | Translation, Rotation, Scale |
| `Color` | FColor | `"0,0,0,255"` | R, G, B, A bytes |
| `LinearColor` | FLinearColor | `"1,1,1,1"` | R, G, B, A floats |

## Containers

Container types use **parentheses** for inner type parameters. Two forms for defaults:

1. **String form** (basic-type containers only): `Default="value1,value2,..."`
2. **Child element form**: element name = inner type in PascalCase

```xml
<!-- String form: basic-type containers -->
<Variable Name="Scores" Type="Array(Float)" Default="1.5,2.0,3.5" />
<Variable Name="Names" Type="Set(String)" Default="Alice,Bob" />

<!-- Element form: any container type -->
<Variable Name="Ids" Type="Array(Integer)">
  <Integer>42</Integer>
  <Integer>99</Integer>
</Variable>
<Variable Name="Flags" Type="Array(Boolean)">
  <Boolean>True</Boolean>
  <Boolean>False</Boolean>
</Variable>
<Variable Name="Points" Type="Array(Vector)">
  <Vector>0,0,0</Vector>
  <Vector>100,200,300</Vector>
</Variable>
<Variable Name="Actors" Type="Array(Object(Actor))">
  <Actor Name="A" />
  <Actor Name="B" />
</Variable>
```

> **Constraint:** Struct/object inner types MUST use child elements; string `Default` is rejected for non-basic inner types.

| Type | Syntax | Notes |
|---|---|---|
| `Array(InnerType)` | `Array(Integer)` | String or children |
| `Set(InnerType)` | `Set(String)` | String or children |
| `Map(KeyType,ValueType)` | `Map(String,Integer)` | Children only |

## Object references

Use `()` after the qualifier prefix. Class names are case-insensitive.

| Type | Example | Description |
|---|---|---|
| `Object(Actor)` | Blueprint path or inline | `UObject*` pointer |
| `Class(Actor)` | Blueprint path | `TSubclassOf` |
| `SoftObject(Texture2D)` | Asset path | `TSoftObjectPtr` |
| `SoftClass(Actor)` | Asset path | `TSoftClassPtr` |

Any unrecognized type is auto-detected as `Object(TypeName)` via `ResolveClass`.

## Enum

Enums are auto-detected — no prefix needed. The `E` prefix is auto-added.

| Type | Example |
|---|---|
| `ECollisionChannel` | `"ECC_WorldStatic"` |

> **Literal braces in Default:** The `Default` attribute treats `{}` as literal text, not a binding expression. Use normal `"value"` syntax for strings containing braces — the binding-expression detection is suppressed for `<Variable Default="...">`.
