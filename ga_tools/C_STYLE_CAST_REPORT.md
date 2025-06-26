# C-Style Cast Report for ga_tools Component

## Summary
The ga_tools component has very few remaining C-style casts. Most have already been converted to static_cast. A total of **13 actual C-style casts** remain in the code (excluding false positives from comments and sizeof expressions).

## Files with C-Style Casts (sorted by count)

### 1. **SingletonGenEngine.cpp** (5 casts)
```cpp
Line 257: measureStep((double) x+2,actualGeneration,actualContext,plotEngine,plotEngineGenContext);
Line 442: for(z=0;z<(int) sizeof(RESTORE_GA_TEMPLATE<int>);++z)
Line 454: for(z=0;z<(int) sizeof(RESTORE_GA_TEMPLATE<int>);++z)
Line 474: for(z=0;z<(int) sizeof(RESTORE_GA_TEMPLATE<int>);++z)
Line 486: for(z=0;z<(int) sizeof(RESTORE_GA_TEMPLATE<int>);++z)
```

### 2. **SelectionStrategies/RandomSelectStrategy.cpp** (3 casts)
```cpp
Line 79:  r1 = ((int) (m_random->rand()*1000000.0))%list.size();
Line 98:  for(int x=0;x<(int) list.size() && x<oldGeneration->getSize()+kill;++x)
Line 115: for(int x=0;x<(int) list.size() && x<newGeneration->getSize();++x)
```

### 3. **MutationFactorStrategies/StandartMutationFactorStrategy.cpp** (3 casts)
```cpp
Line 53: int rand = ((int) (random.rand()*10000))%2;
Line 64: double durch = sum / (double) num;
Line 75: double result = sqrt(sum / (double) (num-1));
```

### 4. **SelectionStrategies/TournamentSelectStrategy.cpp** (2 casts)
```cpp
Line 67: r1 = ((int) (m_random->rand()*1000000.0f)) % num;
Line 70: r2 = ((int) (m_random->rand()*1000000.0f)) % num;
```

### 5. **SingletonIndividualFactory.cpp** (2 casts)
```cpp
Line 79: r1 = ((int) (random->rand()*10000.0))%2;
Line 80: r2 = ((int) (random->rand()*10000.0))%1000;
```

### 6. **simulations/template_cycled_GA_Simulation/main.cpp** (2 casts)
```cpp
Line 213: delete (agent) override;
Line 269: delete (agent) override;
```
Note: These appear to be syntax errors where "override" was incorrectly added after delete statements.

## False Positives Found
- **Gen.h, Generation.h**: Lines with "(int)" are in comments, not actual casts
- **restore.h**: sizeof(int) expressions are not C-style casts
- **Comments in main.cpp files**: "static (CameraManipulator)" are in TODO comments

## Total Count
**17 actual C-style casts** remain in the ga_tools component (15 in core files + 2 syntax errors in simulations).

## Recommendation
All these C-style casts should be replaced with static_cast for C++17 compliance. The casts are primarily:
- `(int)` conversions from random number generation (rand() * factor)
- `(double)` conversions for division operations
- `(int)` conversions from size_t container sizes
- `(int)` conversions from sizeof operations

These are all safe conversions that can be replaced with static_cast<int>() or static_cast<double>().