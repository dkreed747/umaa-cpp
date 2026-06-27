# Vector Projection Formula

The linear projection of a vector $\mathbf{v}$ onto a vector $\mathbf{u}$, denoted as $\text{proj}_{\mathbf{u}}(\mathbf{v})$:

where "$\cdot$" represents the dot product, "$\|\mathbf{u}\|$" is the magnitude of vector $\mathbf{u}$, and the result is a vector representing the projection of $\mathbf{v}$ onto $\mathbf{u}$.

$$
\begin{align*}
\text{proj}_{\mathbf{u}}(\mathbf{v}) &= \frac{\mathbf{v} \cdot \mathbf{u}}{\|\mathbf{u}\|^2} \cdot \mathbf{u} \\
&= \frac{\langle \mathbf{v}, \mathbf{u} \rangle}{\|\mathbf{u}\|^2} \cdot \mathbf{u} \\
&= \frac{\sum_{i=1}^{n} v_i u_i}{\sum_{i=1}^{n} u_i^2} \cdot \begin{bmatrix} u_1 \\ u_2 \\ \vdots \\ u_n \end{bmatrix}
\end{align*}
$$

# Latitude Shift Formula
$$
\begin{align*}
\text{current latitude} + \frac{\text{shift in meters}}{6371000m} * \frac{180}{\pi}
\end{align*}
$$

# Longitude Shift Formula
$$
\begin{gather*}
\text{current longitude} + \frac{\frac{\text{shift in meters}}{6371000m} * \frac{180}{\pi}}{\cos(\text{current latitude} * \pi)}
\end{gather*}
$$

# Heading Between Waypoints Formula
$$
\begin{align*}
&x = \cos(\text{latitude of point 2}) * \sin(\Delta \text{longitude})\\
&y = \cos(\text{latitude of point 1}) * \sin(\text{latitude of point 2}) - \sin(\text{latitude of point 1}) * \cos(\text{latitude of point 2}) * \cos(\Delta \text{longitude})\\
&\text{heading} = \arctan(\frac{y}{x})
\end{align*}
$$

# Haversine Distance Formula

$$
\begin{align*}
&a  = \sin^2(\frac{\Delta \text{latitude}}{2})+\sin^2(\frac{\Delta \text{longitude}}{2}) * \cos(\text{target latitude}) * \cos(\text{current latitude})\\
&c = 2 * \arcsin(\sqrt a)\\
&\text{Haversine Distance} = 637100m * c
\end{align*}
$$
