def max_diag(sq_arr):
    """
    Given a square matrix produces another squared matrix with the same contents, 
    but the columns are re-orered in order to have the highest values in the main diagonal
    Parameter: sq_arr - a squared matrix
    Example:
    In [1]: import numpy as np
            max_diag(np.array([[1,10],[20,2]]))
    Out[1]: array([[10.,  1.],
                   [ 2., 20.]])
    This function is useful to reorder a confusion matrix when the two label vectors
    have different codings
    """
    import numpy as np
    if len(sq_arr.shape) != 2 or sq_arr.shape[0]!=sq_arr.shape[1]:
        return "Not a squared matrix"
    # find the position of the maximum value in each row
    max_pos = [np.argmax(sq_arr[i,:]) for i in range(sq_arr.shape[0])]
    if len(set(max_pos))!=sq_arr.shape[0]:
        return "There are columns with non unique maximum"
    sq_arr_sh = np.empty(sq_arr.shape)
    for i in range(sq_arr.shape[0]):
        sq_arr_sh[:,i] = sq_arr[:,max_pos[i]]
    return(sq_arr_sh)