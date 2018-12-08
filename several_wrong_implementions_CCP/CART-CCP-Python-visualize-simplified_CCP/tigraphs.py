# -*- coding: utf-8 -*-
import numpy as np
import igraph as ig

class BasicNode: #should be interfaced to from a graph object
    def __init__(self, content=None, labels=None):
        self.content = content
        self.incident_edges =set([])
        self.incident_outward_edges=set([])
        self.incident_inward_edges=set([])
        if labels is None:
            self.labels=set([])
        else:
            self.labels=labels
    
    def add_edge(self,Edge): 
        if self in Edge.ends:
            self.incident_edges.add(Edge)
            if Edge.source()==self:
                self.incident_outward_edges.add(Edge)
            else:
                self.incident_inward_edges.add(Edge)
        else:
            print ('Cannot add edge to vertex, vertex not in ends.')
    
    def remove_edge(self,Edge):
        self.incident_edges.discard(Edge)
        
    def get_neighbors(self): 
        neighbors = [Edge.ends for Edge in self.incident_edges]
        unique_neighbors = list(set(reduce(lambda x,y: x+y, neighbors))) 
        if [self, self] not in neighbors: #checks for a loop
            unique_neighbors.remove(self)
        return unique_neighbors
    
    def get_targets(self):
        #targets = set([Edge.target() for Edge in self.incident_outward_edges])
        targets = set(map(lambda x: x.target(), self.incident_outward_edges))
        return targets
   
    def get_sources(self):
        #sources = set([Edge.source() for Edge in self.incident_inward_edges])
        sources = set(map(lambda x: x.source(), self.incident_inward_edges))
        return sources

    def add_label(self, label):
        self.labels.add(label)
        
    def remove_label(self, label):
        self.labels.discard(label)
        
class BasicEdge: #should be interfaced to from a graph object
    def __init__(self, content=None, ends=[], labels=None):
        self.content = content
        self.ends = ends
        if labels is None:
            self.labels=set([])
        else:
            self.labels=labels
               
    def source(self):
        return self.ends[0]
        
    def target(self):
        return self.ends[1]

    def add_label(self, label):
        self.labels.add(label)        
        
    def remove_label(self, label):
        self.labels.discard(label)

def update_up(class_method):
    def inner(self, *args, **kwargs):
        method_name = class_method.func_name
        class_method(self, *args, **kwargs)
        for Supergraph in self.supergraphs:
            getattr(Supergraph, method_name)(*args, **kwargs)
    return inner

def update_up_down(class_method):
    def inner(self, *args, **kwargs):
        method_name = class_method.func_name
        if class_method(self, *args, **kwargs):
            for Supergraph in self.supergraphs:
                getattr(Supergraph, method_name)(*args, **kwargs)
            for Subgraph in self.subgraphs:
                getattr(Subgraph, method_name)(*args, **kwargs) 
    return inner

   
class Graph(object):
    def __init__(self, subgraphs=None, supergraphs=None, vertices=None, edges=None, Vertex=BasicNode, Edge=BasicEdge):
            
        if edges == None:
            edges = []
        self.edges=set(edges)
        if vertices == None:
            vertices = []
        self.vertices=vertices
        if subgraphs == None:
            subgraphs=[]
        self.subgraphs =set(subgraphs)
        if supergraphs == None:
            supergraphs =[]
        self.supergraphs=set(supergraphs)
        for Supergraph in supergraphs:
            Supergraph.add_subgraph(self)
        for Subgraph in subgraphs:
            Subgraph.add_supergraph(self)
        self.vertex_dict = {}
        self.edges_dict={}
        self.Vertex = Vertex
        self.Edge = Edge
        
    @update_up
    def create_vertex(self):
        self.vertices.append(self.Vertex())
        
    def create_vertices(self, no_create):
        for i in range(no_create):
            self.create_vertex()
    
    @update_up
    def add_vertex(self, Vertex):
        if Vertex in self.vertices:
            return
        self.vertices.append(Vertex)
    
    @update_up
    def create_edge(self, ends):
        NewEdge=self.Edge(ends=ends)
        self.edges.add(NewEdge)
        for Vertex in ends:
            Vertex.add_edge(NewEdge)
    
    @update_up_down
    def remove_edge(self, Edge):
        if not Edge in self.edges:
            return False
        self.edges.discard(Edge)
        return True
    
    def get_incident_edges(self, Vertex):
        incident_edges = Vertex.incident_edges & self.edges
        return incident_edges
    
    @update_up_down
    def remove_vertex(self, Vertex):
        if Vertex not in self.vertices:
            return False
        edges_to_remove = self.get_incident_edges(Vertex)
        for Edge in edges_to_remove:
            self.remove_edge(Edge)
        self.vertices.remove(Vertex)
        return True
        
    def get_vertex_neighbors(self, Vertex):
        neighbors = (Vertex.get_neighbors() & set(self.vertices))
        return neighbors
    
    def get_degree(self, Vertex):
        return len(self.get_incident_edges(Vertex))        
        
    def get_number_vertices(self):
        return len(self.vertices)
    
    def get_number_edges(self):
        return len(self.edges)
  
    def get_adjacency_matrix(self):
        #adj_list = [self.get_adjacency_list_of_vertex(Vertex) for Vertex in self.vertices]
        adj_list = map(lambda x: self.get_adjacency_list_of_vertex(x), self.vertices)
        adj_mat = np.array(adj_list)
        return adj_mat
            
    def get_adjacency_matrix_as_list(self):
        return self.get_adjacency_matrix().tolist()
    
    def set_adjacency_list(self, adj_list):
        self.vertices=[]
        self.edges=[]
        
    def add_subgraph(self,Subgraph):
        self.subgraphs.add(Subgraph)
        
    def add_supergraph(self, Supergraph):
        self.supergraphs.add(Supergraph)
        
    def is_in(self,vertex_or_edge):
        if (vertex_or_edge in self.edges) or (vertex_or_edge in self.vertices):
            return True
        else:
            return False
                    

    def get_incident_outward_edges(self,Vertex):
        return (Vertex.incident_outward_edges & self.edges)
        
    def get_incident_inward_edges(self,Vertex):
        return (Vertex.incident_inward_edges & self.edges)
        
    def get_vertex_targets(self, Vertex):
        targets = (Vertex.get_targets() & set(self.vertices))
        return targets
        
    def get_vertex_sources(self, Vertex):
        sources = (Vertex.get_sources() & set(self.vertices))
        return sources
        
    def add_vertex_label(self, vertex, label):
        self.vertex_dict[label] = vertex
        vertex.add_label(label)
    
    def get_vertex(self,label):
        if label in self.vertex_dict.keys():
            return self.vertex_dict[label]
        else:
            return None
            
    def get_vertex_label(self, vertex):
        labels = vertex.get_labels()
        labels = labels & self.vertex_dict.keys()
        labels = filter(lambda x: self.get_vertex[x]==vertex,labels)
        
    def remove_vertex_label(self, label):
        vertex=self.vertex_dict.pop(label, 'Not Found')
        if vertex == 'Not Found':
            return
        else:
            vertex.remove_label(label)
        
    def add_edge_label(self, edge, label):
        self.edge_dict[label] = edge
        edge.add_label(label)
    
    def get_edge(self,label):
        if label in self.edge_dict.keys():
            return self.edge_dict[label]
        else:
            return None
            
    def get_edge_label(self, edge):
        labels = edge.get_labels()
        labels = labels & self.edge_dict.keys()
        labels = filter(lambda x: self.get_edge[x]==edge,labels)
        
    def remove_edge_label(self, label):
        edge=self.edge_dict.pop(label, 'Not Found')
        if edge == 'Not Found':
            return
        else:
            edge.remove_label(label)
        
        

class UnDirGraph(Graph, object):
 
    def get_adjacency_list_of_vertex(self, Vertex):
            N = self.get_number_vertices()
            adj_list= [0 for x in range(N)]
            incident_edges = self.get_incident_edges(Vertex)
            for Edge in incident_edges:
                ends = Edge.ends
                if ends[0] != Vertex:
                    index = self.vertices.index(ends[0])
                else:
                    index = self.vertices.index(ends[1])
                adj_list[index] += 1
            return adj_list   
    
    def set_adjacency_matrix(self, adj_mat): 
        shape = np.shape(adj_mat)
        if shape[0] != shape[1]:
            print ('Wrong shape, expecting square matrix.')
            return
        n = shape[0]
        self.vertices=[]
        self.edges=[]
        self.create_vertices(n)
        for row in range(n):
            Source = self.vertices[row]
            for col  in range(row+1):
                no_edges = adj_mat[row, col]
                Target = self.vertices[col]
                for Edge in range(no_edges):
                    self.create_edge(ends=[Source, Target])
                    
    def plot(self):
        A = self.get_adjacency_matrix_as_list()
        convert_to_igraph = ig.Graph.Adjacency(A, mode='undirected')
        ig.plot(convert_to_igraph)

class DirGraph(Graph):
    def get_incident_outward_edges(self,Vertex):
        return (Vertex.incident_outward_edges & self.edges)
        
    def get_incident_inward_edges(self,Vertex):
        return (Vertex.incident_inward_edges & self.edges)
    
    def get_adjacency_list_of_vertex(self, Vertex):
        N = self.get_number_vertices()
        adj_list= [0 for x in range(N)]
        incident_edges = self.get_incident_outward_edges(Vertex)
        for Edge in incident_edges:
            target = Edge.target()
            index = self.vertices.index(target)
            adj_list[index] += 1
        return adj_list 
    
    def set_adjacency_matrix(self, adj_mat): 
        shape = np.shape(adj_mat)
        if shape[0] != shape[1]:
            print ('Wrong shape, expecting square matrix.')
            return
        n = shape[0]
        self.vertices=[]
        self.edges=[]
        self.create_vertices(n)
        for row, col in range(n):
            no_edges = adj_mat[row, col]
            Source = self.vertices[row]
            Target = self.vertices[col]
            for Edge in range(no_edges):
                self.create_edge(ends=[Source, Target]) 

    def get_vertex_targets(self, Vertex):
        targets = (Vertex.get_targets() & set(self.vertices))
        return targets
        
    def get_vertex_sources(self, Vertex):
        sources = (Vertex.get_sources() & set(self.vertices))
        return sources
        
    def plot(self):
        A = self.get_adjacency_matrix_as_list()
        convert_to_igraph = ig.Graph.Adjacency(A)
        ig.plot(convert_to_igraph)

#This is a wrapper to a class definition, deciding whether to inherit
#from DirGraph or UnDirGraph at runtime. It can be initialised by
#number of vertices or the number of edges.

def return_linear_class(directed=False):
    if directed:
        base=DirGraph
    else:
        base=UnDirGraph
    class Linear(base, object): 
        def __init__(self, number_vertices=0, number_edges=0, **kwargs):
            super(Linear, self).__init__(**kwargs)
            self.linear_generate(number_vertices, number_edges)
    
        def linear_generate(self,number_vertices, number_edges):
            if (not number_edges ==0) and (not number_vertices==0):
                if not number_vertices == number_edges+1:
                    print('Number of edges and vertices incompatible!')
                    return
                else:
                    self.number_vertices=number_vertices
                    
            elif not number_edges==0:
                self.number_vertices = number_edges +1
            else:
                self.number_vertices = number_vertices
                            
            self.create_vertices(self.number_vertices)
            for index in range(self.number_vertices -1):
                Source = self.vertices[index]
                Target = self.vertices[index+1]
                self.create_edge([Source, Target])
    return Linear

#instantiates the Linear class
def create_linear(directed=False, number_vertices=0, number_edges=0,**kwargs):
    linear = return_linear_class(directed)(number_vertices, number_edges,**kwargs)
    return linear

#Class definition wrapper to dynamically inherti from DirGraph or UnDirGraph.
#Also has a composition from Linear, to create a cycle it joins the ends
#of a linear graph.
def return_cycle_class(directed=False):
    if directed:
        base = DirGraph
    else:
        base = UnDirGraph
    class Cycle(base, object):
        def __init__(self, number_vertices=0, number_edges=0, **kwargs):
            super(Cycle, self).__init__(**kwargs)
            if (not number_edges==0) and (not number_vertices==0):
                if not number_edges == number_vertices:
                    print ('Numbers of edges and vertices incompatible!')
                    return
            elif not number_edges ==0:
                number_vertices = number_edges
            Linear_part = create_linear()
            Linear_part.linear_generate(number_vertices, number_edges-1)
            
            self.vertices = Linear_part.vertices
            self.edges = Linear_part.edges
            self.cycle_generate(number_vertices)
                 
        def cycle_generate(self, number_vertices):
            Target = self.vertices[0]
            Source = self.vertices[number_vertices-1]
            self.create_edge(ends=[Source, Target])
    return Cycle

def create_cycle(directed=False, number_vertices=0, number_edges=0, **kwargs):
    cycle = return_cycle_class(directed)(number_vertices, number_edges, **kwargs)
    return cycle

class Complete(UnDirGraph, object):
    def __init__(self,number_vertices=0, **kwargs):
        super(Complete, self).__init__(**kwargs)
        self.create_vertices(no_create=number_vertices)
        ends=[]
        for Source in self.vertices:
            for Target in self.vertices:
                if [Source,Target] not in ends:
                    if not Source == Target:
                        self.create_edge(ends=[Source,Target])
                        ends.append([Source,Target])
                        ends.append([Target, Source])


def return_tree_class(directed=False):
    if directed:
        base = DirGraph
    else:
        base = UnDirGraph
                          
    class Tree(base, object):
        def __init__(self, **kwargs):
            super(Tree, self).__init__(**kwargs)
            self.leaves=set([])
            self.find_leaves()
            
        def is_leaf(self, vertex):
           
            if self.get_degree(vertex) == 1:
                return True
            elif self.get_number_vertices() ==1:
                return True
            else:
                return False
                
        def set_root(self, vertex):
            if vertex in self.vertices:
                self.remove_vertex_label('Root')
                self.add_vertex_label(vertex, label='Root')
        
        def get_root(self):
            return self.get_vertex('Root')
        
        def find_leaves(self):
            self.leaves = set(filter(self.is_leaf,self.vertices))
            return [leaf for leaf in self.leaves]
    return Tree

def create_tree(directed=False, **kwargs):
    tree = return_tree_class(directed)(**kwargs)
    return tree

def return_nary_tree_class(directed=False):
    base = return_tree_class(directed)
    class NaryRootedTree(base,object):
        def __init__(self, N=0, **kwargs):
            super(NaryRootedTree, self).__init__(**kwargs)
            self.N = N

        def split_vertex(self, vertex):
            if vertex in self.leaves:
                
                children = [self.Vertex() for i in range(self.N)]
                vertex.children = children
                self.leaves.discard(vertex)
                for Child in children:
                    self.add_vertex(Child)
                    self.leaves.add(Child)
                    Child.parent=vertex
                    self.create_edge(ends=[vertex, Child])


        def fuse_vertex(self, vertex):
            self.leaves.add(vertex)
            try:
                children=vertex.children
            except AttributeError:
                return
            if children==None:
                return
            
            for child in children:
                self.fuse_vertex(child)
                self.leaves.discard(child)
                self.remove_vertex(child)
                child.parent = None
                vertex.children = None

        def create_full_n_level(self, n):
            self.vertices =[]
            self.edges =set([])
            self.create_vertex()
            self.set_root(self.vertices[0])
            for level in range(n):
                leaves=self.find_leaves()
                for Leaf in leaves:
                    self.split_vertex(Leaf)

        def get_descendants(self, node, desc=set({})):
            try:
                children=node.children
            except AttributeError:
                node.children=None
            if children != None:
                for child in children:
                    desc = desc.union(set({child}))
                    desc= desc.union(self.get_descendants(child))
                return desc
            else:
                return desc
      
    return NaryRootedTree
    
def create_nary_tree(directed=False, N=0,**kwargs):
    nary_tree = return_nary_tree_class(directed)(N, **kwargs)
    return nary_tree
    

         
                
