#!/usr/bin/env python3
"""Fix esn.h issues"""

import re

def fix_esn():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/esn.h"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix the ESNConf structure - add missing members
    content = re.sub(
        r'struct ESNConf\s*{\s*/// switch on to get the internal weights and activations inspectabe\s*};',
        '''struct ESNConf {
  int numNeurons = 100;
  double inputStrength = 0.1;
  double inputRatio = 1.0;
  double connectionRatio = 0.1;
  double spectralRadius = 0.9;
  bool inspectInternals = false;
  double learningrate = 0.01;
};''',
        content
    )
    
    # Fix method parameter with override
    content = re.sub(r'const matrix::Matrix& x overridesi\)', 
                     r'const matrix::Matrix& xsi)', content)
    
    # Fix storestatic_cast -> store
    content = re.sub(r'virtual bool storestatic_cast<FILE\*>\(const\);', 
                     r'virtual bool store(FILE* f) const;', content)
    
    # Fix restorestatic_cast -> restore
    content = re.sub(r'virtual bool restorestatic_cast<FILE\*>;', 
                     r'virtual bool restore(FILE* f);', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_esn()