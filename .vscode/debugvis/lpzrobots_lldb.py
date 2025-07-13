#!/usr/bin/env python3
"""
LPZRobots LLDB Debug Formatters for macOS
Provides custom debug visualizations for Matrix, Robot, and Controller classes
"""

import lldb
import re
from typing import Optional, List, Tuple

def __lldb_init_module(debugger: lldb.SBDebugger, internal_dict: dict):
    """Initialize the LLDB formatters"""
    debugger.HandleCommand('type summary add -F lpzrobots_lldb.matrix_summary matrix::Matrix')
    debugger.HandleCommand('type synthetic add -l lpzrobots_lldb.MatrixSynthProvider matrix::Matrix')
    
    debugger.HandleCommand('type summary add -F lpzrobots_lldb.oderobot_summary "lpzrobots::OdeRobot"')
    debugger.HandleCommand('type synthetic add -l lpzrobots_lldb.OdeRobotSynthProvider "lpzrobots::OdeRobot"')
    
    debugger.HandleCommand('type summary add -F lpzrobots_lldb.controller_summary -x ".*Controller"')
    debugger.HandleCommand('type synthetic add -l lpzrobots_lldb.ControllerSynthProvider -x ".*Controller"')
    
    debugger.HandleCommand('type summary add -F lpzrobots_lldb.pos_summary Pos')
    debugger.HandleCommand('type summary add -F lpzrobots_lldb.color_summary Color')
    
    print("LPZRobots LLDB formatters loaded successfully!")


def matrix_summary(valobj: lldb.SBValue, internal_dict: dict) -> str:
    """Summary formatter for Matrix class"""
    try:
        m = valobj.GetChildMemberWithName('m').GetValueAsUnsigned()
        n = valobj.GetChildMemberWithName('n').GetValueAsUnsigned()
        
        if m == 0 or n == 0:
            return f"Matrix[{m}x{n}] (empty)"
            
        data_ptr = valobj.GetChildMemberWithName('data')
        
        # For small matrices, show preview
        if m * n <= 9:
            values = []
            error = lldb.SBError()
            
            # data_ptr is a pointer to double, we need to read the actual values
            for i in range(m * n):
                # Read 8 bytes (double) at offset i * 8
                val_data = data_ptr.GetPointeeData(i, 1)
                if val_data.IsValid():
                    val = val_data.GetDouble(error, 0)
                    if error.Success():
                        values.append(val)
                    else:
                        values.append(0.0)
                else:
                    values.append(0.0)
                
            # Format as matrix
            result = f"Matrix[{m}x{n}] = ["
            for i in range(m):
                if i > 0:
                    result += "; "
                for j in range(n):
                    if j > 0:
                        result += " "
                    result += f"{values[i*n + j]:.3g}"
            result += "]"
            return result
        else:
            return f"Matrix[{m}x{n}]"
            
    except Exception as e:
        return f"Matrix (error: {e})"


class MatrixSynthProvider:
    """Synthetic children provider for Matrix class"""
    
    def __init__(self, valobj: lldb.SBValue, internal_dict: dict):
        self.valobj = valobj
        self.update()
        
    def update(self):
        try:
            self.m = self.valobj.GetChildMemberWithName('m').GetValueAsUnsigned()
            self.n = self.valobj.GetChildMemberWithName('n').GetValueAsUnsigned()
            self.data = self.valobj.GetChildMemberWithName('data')
        except Exception:
            self.m = self.n = 0
            self.data = None
            
    def num_children(self) -> int:
        # Show dimensions + up to 10 rows
        return 3 + min(self.m, 10) if self.m > 0 else 3
        
    def get_child_at_index(self, index: int) -> Optional[lldb.SBValue]:
        if index == 0:
            return self.valobj.CreateValueFromExpression('rows', f'{self.m}')
        elif index == 1:
            return self.valobj.CreateValueFromExpression('cols', f'{self.n}')
        elif index == 2:
            return self.valobj.CreateValueFromExpression('size', f'{self.m * self.n}')
        else:
            # Row data
            row_idx = index - 3
            if row_idx < self.m and self.data:
                # Create a synthetic array view of the row
                # data is already a pointer, so we just need to add the offset
                row_expr = f'((double*){self.data.GetLoadAddress()} + {row_idx * self.n})'
                row_type = self.valobj.GetTarget().FindFirstType('double').GetArrayType(self.n)
                return self.valobj.CreateValueFromAddress(f'row[{row_idx}]', 
                                                         self.data.GetLoadAddress() + row_idx * self.n * 8,
                                                         row_type)
                
        return None
        
    def get_child_index(self, name: str) -> int:
        if name == 'rows':
            return 0
        elif name == 'cols':
            return 1
        elif name == 'size':
            return 2
        else:
            match = re.match(r'row\[(\d+)\]', name)
            if match:
                return 3 + int(match.group(1))
        return -1


def oderobot_summary(valobj: lldb.SBValue, internal_dict: dict) -> str:
    """Summary formatter for OdeRobot class"""
    try:
        name = valobj.GetChildMemberWithName('name')
        name_str = name.GetSummary() if name else "Unknown"
        
        # Get sensor and motor counts from lists
        sensors_list = valobj.GetChildMemberWithName('sensors')
        motors_list = valobj.GetChildMemberWithName('motors')
        
        # Count items in std::list - this is tricky in LLDB
        sensor_count = 0
        motor_count = 0
        
        # For now, just indicate presence
        if sensors_list and sensors_list.IsValid():
            sensor_count = "has"
        if motors_list and motors_list.IsValid():
            motor_count = "has"
            
        initialized = valobj.GetChildMemberWithName('initialized')
        init_str = " (initialized)" if initialized and initialized.GetValueAsUnsigned() else " (not initialized)"
        
        return f"OdeRobot {name_str}{init_str}"
        
    except Exception as e:
        return f"OdeRobot (error: {e})"


class OdeRobotSynthProvider:
    """Synthetic children provider for OdeRobot class"""
    
    def __init__(self, valobj: lldb.SBValue, internal_dict: dict):
        self.valobj = valobj
        self.update()
        
    def update(self):
        try:
            self.name = self.valobj.GetChildMemberWithName('name')
            self.initialized = self.valobj.GetChildMemberWithName('initialized')
            self.objects = self.valobj.GetChildMemberWithName('objects')
            self.joints = self.valobj.GetChildMemberWithName('joints')
            self.sensors_list = self.valobj.GetChildMemberWithName('sensors')
            self.motors_list = self.valobj.GetChildMemberWithName('motors')
        except Exception:
            self.name = None
            self.initialized = None
            self.objects = None
            self.joints = None
            self.sensors_list = None
            self.motors_list = None
            
    def num_children(self) -> int:
        # name, initialized, primitives, joints, sensors, motors
        return 6
        
    def get_child_at_index(self, index: int) -> Optional[lldb.SBValue]:
        if index == 0:
            return self.name
        elif index == 1:
            return self.initialized
        elif index == 2:
            return self.objects
        elif index == 3:
            return self.joints
        elif index == 4:
            return self.sensors_list
        elif index == 5:
            return self.motors_list
                
        return None


def controller_summary(valobj: lldb.SBValue, internal_dict: dict) -> str:
    """Summary formatter for Controller classes"""
    try:
        type_name = valobj.GetTypeName().split("::")[-1]
        
        sensors = valobj.GetChildMemberWithName('number_sensors')
        motors = valobj.GetChildMemberWithName('number_motors')
        t = valobj.GetChildMemberWithName('t')
        
        sensor_num = sensors.GetValueAsUnsigned() if sensors else 0
        motor_num = motors.GetValueAsUnsigned() if motors else 0
        t_val = t.GetValueAsUnsigned() if t else 0
        
        return f"{type_name} [{sensor_num} sensors, {motor_num} motors, t={t_val}]"
        
    except Exception as e:
        return f"Controller (error: {e})"


class ControllerSynthProvider:
    """Synthetic children provider for Controller classes"""
    
    def __init__(self, valobj: lldb.SBValue, internal_dict: dict):
        self.valobj = valobj
        self.matrix_members = []
        self.update()
        
    def update(self):
        # Find all matrix members
        self.matrix_members = []
        for name in ['A', 'C', 'S', 'L', 'h', 'b', 'R', 'x_buffer', 'y_buffer']:
            member = self.valobj.GetChildMemberWithName(name)
            if member and member.IsValid():
                self.matrix_members.append(name)
                
    def num_children(self) -> int:
        # Basic info + matrices
        return 3 + len(self.matrix_members)
        
    def get_child_at_index(self, index: int) -> Optional[lldb.SBValue]:
        if index == 0:
            return self.valobj.GetChildMemberWithName('number_sensors')
        elif index == 1:
            return self.valobj.GetChildMemberWithName('number_motors')
        elif index == 2:
            return self.valobj.GetChildMemberWithName('t')
        else:
            matrix_idx = index - 3
            if matrix_idx < len(self.matrix_members):
                return self.valobj.GetChildMemberWithName(self.matrix_members[matrix_idx])
                
        return None


def pos_summary(valobj: lldb.SBValue, internal_dict: dict) -> str:
    """Summary formatter for Pos class"""
    try:
        x = valobj.GetChildMemberWithName('x').GetData().GetDouble(lldb.SBError(), 0)
        y = valobj.GetChildMemberWithName('y').GetData().GetDouble(lldb.SBError(), 0)
        z = valobj.GetChildMemberWithName('z').GetData().GetDouble(lldb.SBError(), 0)
        return f"({x:.3f}, {y:.3f}, {z:.3f})"
    except Exception:
        return "Pos(?)"


def color_summary(valobj: lldb.SBValue, internal_dict: dict) -> str:
    """Summary formatter for Color class"""
    try:
        r = valobj.GetChildMemberWithName('r').GetData().GetDouble(lldb.SBError(), 0)
        g = valobj.GetChildMemberWithName('g').GetData().GetDouble(lldb.SBError(), 0)
        b = valobj.GetChildMemberWithName('b').GetData().GetDouble(lldb.SBError(), 0)
        a = valobj.GetChildMemberWithName('a').GetData().GetDouble(lldb.SBError(), 0)
        return f"RGBA({r:.2f}, {g:.2f}, {b:.2f}, {a:.2f})"
    except Exception:
        return "Color(?)"