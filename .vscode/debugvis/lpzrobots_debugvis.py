#!/usr/bin/env python3
"""
LPZRobots Debug Visualizers for VSCode
Provides custom debug visualizations for Matrix, Robot, and Controller classes
"""

import gdb
import re
import sys
from typing import Any, Dict, List, Optional, Tuple

# Enable pretty printing
gdb.execute("set print pretty on")


class MatrixPrinter:
    """Pretty printer for matrix::Matrix class"""
    
    def __init__(self, val: gdb.Value):
        self.val = val
        
    def to_string(self) -> str:
        """Return a string representation of the matrix"""
        try:
            m = int(self.val['m'])
            n = int(self.val['n'])
            
            if m == 0 or n == 0:
                return f"Matrix[{m}x{n}] (empty)"
                
            # For small matrices, show the full content
            if m * n <= 16:
                return self._format_full_matrix(m, n)
            else:
                # For large matrices, show summary
                return self._format_summary_matrix(m, n)
                
        except Exception as e:
            return f"Matrix (error accessing data: {e})"
    
    def _format_full_matrix(self, m: int, n: int) -> str:
        """Format small matrices with full content"""
        try:
            data_ptr = self.val['data']
            
            result = f"Matrix[{m}x{n}]:\n"
            
            # Find maximum width for alignment
            max_width = 0
            values = []
            for i in range(m):
                row = []
                for j in range(n):
                    val = float(data_ptr[i * n + j])
                    row.append(val)
                    max_width = max(max_width, len(f"{val:.4g}"))
                values.append(row)
            
            # Format the matrix
            for i, row in enumerate(values):
                if i == 0:
                    result += "⎡ "
                elif i == m - 1:
                    result += "⎣ "
                else:
                    result += "⎢ "
                    
                for j, val in enumerate(row):
                    result += f"{val:>{max_width}.4g}"
                    if j < n - 1:
                        result += "  "
                        
                if i == 0:
                    result += " ⎤"
                elif i == m - 1:
                    result += " ⎦"
                else:
                    result += " ⎥"
                    
                if i < m - 1:
                    result += "\n"
                    
            return result
            
        except Exception as e:
            return f"Matrix[{m}x{n}] (error formatting: {e})"
    
    def _format_summary_matrix(self, m: int, n: int) -> str:
        """Format large matrices with summary statistics"""
        try:
            data_ptr = self.val['data']
            
            # Calculate statistics
            min_val = float('inf')
            max_val = float('-inf')
            sum_val = 0.0
            
            for i in range(m * n):
                val = float(data_ptr[i])
                min_val = min(min_val, val)
                max_val = max(max_val, val)
                sum_val += val
                
            mean_val = sum_val / (m * n)
            
            # Show corners
            result = f"Matrix[{m}x{n}]:\n"
            result += f"  min: {min_val:.4g}, max: {max_val:.4g}, mean: {mean_val:.4g}\n"
            result += "  Preview (corners):\n"
            
            # Top-left
            result += f"  [{float(data_ptr[0]):.4g}, {float(data_ptr[1]):.4g}, ..., {float(data_ptr[n-1]):.4g}]\n"
            result += "  [ ... ]\n"
            # Bottom-left  
            result += f"  [{float(data_ptr[(m-1)*n]):.4g}, {float(data_ptr[(m-1)*n + 1]):.4g}, ..., {float(data_ptr[m*n - 1]):.4g}]"
            
            return result
            
        except Exception as e:
            return f"Matrix[{m}x{n}] (error summarizing: {e})"
    
    def children(self) -> List[Tuple[str, Any]]:
        """Provide child elements for expansion in debugger"""
        try:
            m = int(self.val['m'])
            n = int(self.val['n'])
            
            result = [
                ('rows', m),
                ('cols', n),
                ('size', m * n),
            ]
            
            if m * n > 0 and m * n <= 100:
                # For reasonable sized matrices, allow row-by-row inspection
                data_ptr = self.val['data']
                for i in range(min(m, 10)):  # Limit to first 10 rows
                    row_str = "["
                    for j in range(min(n, 10)):  # Limit to first 10 columns
                        if j > 0:
                            row_str += ", "
                        row_str += f"{float(data_ptr[i * n + j]):.4g}"
                    if n > 10:
                        row_str += ", ..."
                    row_str += "]"
                    result.append((f'row[{i}]', row_str))
                    
                if m > 10:
                    result.append(('...', '(more rows)'))
                    
            return result
            
        except Exception:
            return []
    
    def display_hint(self) -> str:
        return 'matrix'


class OdeRobotPrinter:
    """Pretty printer for OdeRobot class"""
    
    def __init__(self, val: gdb.Value):
        self.val = val
        
    def to_string(self) -> str:
        """Return a string representation of the robot"""
        try:
            name = self.val['name']
            # Get sensor and motor counts from the lists
            sensors_list = self.val['sensors']
            motors_list = self.val['motors']
            
            # Since these are std::list, we need to count them
            sensor_num = 0
            motor_num = 0
            
            try:
                # Try to access size() method if available
                sensor_num = int(sensors_list['_M_impl']['_M_node']['_M_size'])
            except:
                # Fallback: count manually by iterating through list
                pass
                
            try:
                motor_num = int(motors_list['_M_impl']['_M_node']['_M_size'])
            except:
                pass
            
            # Try to get position if available
            pos_str = ""
            try:
                # This might vary based on robot implementation
                primitives = self.val['primitives']
                if primitives:
                    # Get position of first primitive (usually main body)
                    first_prim = primitives[0]
                    if first_prim:
                        pos = first_prim['pos']
                        pos_str = f" at ({float(pos['x']):.2f}, {float(pos['y']):.2f}, {float(pos['z']):.2f})"
            except Exception:
                pass
                
            return f"OdeRobot '{name}' [{sensor_num} sensors, {motor_num} motors]{pos_str}"
            
        except Exception as e:
            return f"OdeRobot (error: {e})"
    
    def children(self) -> List[Tuple[str, Any]]:
        """Provide child elements for expansion"""
        try:
            result = []
            
            # Basic info
            result.append(('name', self.val['name']))
            result.append(('initialized', self.val['initialized']))
            
            # Objects (primitives)
            try:
                objects = self.val['objects']
                result.append(('primitives', objects))
            except Exception:
                pass
                
            # Joints
            try:
                joints = self.val['joints']
                result.append(('joints', joints))
            except Exception:
                pass
                
            # Sensors list
            try:
                sensors_list = self.val['sensors']
                result.append(('sensors', sensors_list))
            except Exception:
                pass
                
            # Motors list
            try:
                motors_list = self.val['motors']
                result.append(('motors', motors_list))
            except Exception:
                pass
                
            return result
            
        except Exception:
            return []
            
    def display_hint(self) -> str:
        return 'robot'


class AbstractControllerPrinter:
    """Pretty printer for AbstractController and derived classes"""
    
    def __init__(self, val: gdb.Value):
        self.val = val
        
    def to_string(self) -> str:
        """Return a string representation of the controller"""
        try:
            # Get the actual type name (might be derived class)
            type_name = str(self.val.type).split("::")[-1].rstrip(" *")
            
            sensor_num = motor_num = 0
            try:
                sensor_num = int(self.val['number_sensors'])
                motor_num = int(self.val['number_motors'])
            except Exception:
                pass
                
            # Try to get time step
            t = ""
            try:
                t_val = int(self.val['t'])
                t = f", t={t_val}"
            except Exception:
                pass
                
            return f"{type_name} [{sensor_num} sensors, {motor_num} motors{t}]"
            
        except Exception as e:
            return f"Controller (error: {e})"
            
    def children(self) -> List[Tuple[str, Any]]:
        """Provide child elements for expansion"""
        try:
            result = []
            
            # Basic info
            try:
                result.append(('sensors', self.val['number_sensors']))
                result.append(('motors', self.val['number_motors']))
                result.append(('time_step', self.val['t']))
            except Exception:
                pass
                
            # Common matrices in homeokinetic controllers
            matrix_names = ['A', 'C', 'S', 'L', 'h', 'b', 'y_buffer', 'x_buffer']
            for name in matrix_names:
                try:
                    mat = self.val[name]
                    if mat:
                        result.append((name, mat))
                except Exception:
                    pass
                    
            # Parameters
            try:
                param_list = self.val['parameterlist']
                if param_list:
                    result.append(('parameters', 'Configurable'))
            except Exception:
                pass
                
            return result
            
        except Exception:
            return []
            
    def display_hint(self) -> str:
        return 'controller'


class PosPrinter:
    """Pretty printer for Pos (3D position) class"""
    
    def __init__(self, val: gdb.Value):
        self.val = val
        
    def to_string(self) -> str:
        """Return a string representation of the position"""
        try:
            x = float(self.val['x'])
            y = float(self.val['y']) 
            z = float(self.val['z'])
            return f"Pos({x:.3f}, {y:.3f}, {z:.3f})"
        except Exception as e:
            return f"Pos (error: {e})"
            
    def display_hint(self) -> str:
        return 'array'


class ColorPrinter:
    """Pretty printer for Color class"""
    
    def __init__(self, val: gdb.Value):
        self.val = val
        
    def to_string(self) -> str:
        """Return a string representation of the color"""
        try:
            r = float(self.val['r'])
            g = float(self.val['g'])
            b = float(self.val['b'])
            a = float(self.val['a'])
            return f"Color(r={r:.2f}, g={g:.2f}, b={b:.2f}, a={a:.2f})"
        except Exception as e:
            return f"Color (error: {e})"
            
    def display_hint(self) -> str:
        return 'array'


def lookup_type(val: gdb.Value) -> Optional[object]:
    """Look up a pretty printer for the given value"""
    
    # Get the type name
    type_str = str(val.type)
    
    # Matrix types
    if 'matrix::Matrix' in type_str:
        return MatrixPrinter(val)
        
    # Robot types
    if 'OdeRobot' in type_str or 'lpzrobots::OdeRobot' in type_str:
        return OdeRobotPrinter(val)
        
    # Controller types
    if 'AbstractController' in type_str or 'Controller' in type_str:
        return AbstractControllerPrinter(val)
        
    # Position type
    if type_str.endswith('Pos'):
        return PosPrinter(val)
        
    # Color type
    if type_str.endswith('Color'):
        return ColorPrinter(val)
        
    return None


def register_printers():
    """Register all LPZRobots pretty printers"""
    gdb.pretty_printers.append(lookup_type)


# Register printers when module is loaded
register_printers()

print("LPZRobots debug visualizers loaded successfully!")
print("Supported types: Matrix, OdeRobot, AbstractController, Pos, Color")