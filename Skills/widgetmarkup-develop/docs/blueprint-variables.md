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

Container types use **parentheses** for inner type parameters. **Children-only** — a string `Default` attribute on a container is rejected (route A):

```xml
<Variable Name="Ids" Type="Array(Integer)">
  <Integer>42</Integer>
  <Integer>99</Integer>
</Variable>
<Variable Name="Weights" Type="Array(Float)">
  <Float>1.0</Float><Float>2.0</Float>
</Variable>
<Variable Name="Points" Type="Array(Vector2D)">
  <Vector2D X="1" Y="2" /><Vector2D X="3" Y="4" />
</Variable>
<Variable Name="Tags" Type="Set(String)">
  <String>red</String><String>blue</String>
</Variable>
<Variable Name="Scores" Type="Map(String,Integer)">
  <Pair Key="a" Value="1" /><Pair Key="b" Value="2" />
</Variable>
```

Child elements are the inner type in PascalCase (Basic / Struct / Object). Map uses `<Pair Key=".." Value=".."/>` children.

| Type | Syntax | Notes |
|---|---|---|
| `Array(InnerType)` | `Array(Integer)` | Children only |
| `Set(InnerType)` | `Set(String)` | Children only |
| `Map(KeyType,ValueType)` | `Map(String,Integer)` | `<Pair>` children |

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

> **Use the full value name** (e.g. `ECC_WorldStatic`, `HAlign_Center`). DisplayName shorthand is **disabled** — enum matching is value-name exact only.

> **Literal braces in Default:** The `Default` attribute treats `{}` as literal text, not a binding expression. Use normal `"value"` syntax for strings containing braces — the binding-expression detection is suppressed for `<Variable Default="...">`.
