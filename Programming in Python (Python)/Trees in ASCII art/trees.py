#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def render_tree(tree: list = None, indent: int = 2, separator: str = ' ', depth_last: list = None, ongoing_lines: list = None) -> str:
    """
    ChatGPT:
        Renders a nested list as a tree-like ASCII structure.

        Args:
            tree (list): The list to render as a tree structure.
            indent (int): Number of spaces for indentation per level.
            separator (str): Character used for spacing.
            depth (int): The current depth level (used for recursion).
            last (bool): Indicates if the current node is the last child.
            ongoing_lines (list): Tracks ongoing lines for visual structure.

        Returns:
            str: A string representation of the tree structure.

    """

    # ENUMS
    ONLY_OTHER = 1
    ONLY_LISTS = 2
    BOTH = 3
    EMPTY = 0

    # Helper functions
    def listtype(tree: list, depth: int = 0) -> list:
        isList = False
        isOther = False
        index = -1
        for i, _ in enumerate(tree):
            if isinstance(tree[i], list):
                isList = True
            else:
                if isOther and depth == 0:
                    raise Exception('Invalid tree')
                isOther = True
                index = i
        if depth == 0 and not isList:
            raise Exception('Invalid tree')
        if isList and isOther:
            return [BOTH, index]
        if isList:
            return [ONLY_LISTS, index]
        if isOther:
            return [ONLY_OTHER, index]
        return [EMPTY, index]

    # Builds node string
    def printnode(node, indent: int = 2, separator: str = ' ', depth_last: list = None, ongoing_lines: list = None) -> str:
        node_str = ""
        node_str += ''.join(('│' + (indent - 1) * separator if i in ongoing_lines else indent * separator)for i in range(depth_last[0] - 1))
        if depth_last[0] > 0:
            line = '└' if depth_last[1] else '├'
            node_str += line + (indent - 2) * '─' + '>'
        node_str += str(node)
        return node_str + '\n'

    # Check tree validity
    if not isinstance(tree, list) or tree is None or indent < 2:
        raise Exception('Invalid tree')

    # Initialize ongoing_lines and depth_last if None
    if depth_last is None:
        depth_last = [0, False]
        ongoing_lines = []

    tree_str = ""

    # Determine tree type
    opt = listtype(tree, depth_last[0])

    if opt[0] == BOTH:
        tree_str += printnode(tree[opt[1]], indent, separator, depth_last, ongoing_lines)
        if depth_last[1] and (depth_last[0] - 1) in ongoing_lines:
            ongoing_lines.remove(depth_last[0] - 1)

        tree.pop(opt[1])

        next_depth_last = [depth_last[0] + 1, listtype(tree[0], depth_last[0] + 1)[0] == BOTH]
        if not next_depth_last[1] and depth_last[0] not in ongoing_lines:
            ongoing_lines.append(depth_last[0])
        tree_str += render_tree(tree[0], indent, separator, next_depth_last, ongoing_lines)


    elif opt[0] == ONLY_LISTS:
        for item in tree:
            tree_str += render_tree(item, indent, separator, [depth_last[0], item == tree[-1]], ongoing_lines)

    elif opt[0] == ONLY_OTHER:
        for item in tree:
            tree_str += printnode(item, indent, separator,[depth_last[0], item == tree[-1]], ongoing_lines)

    return tree_str
