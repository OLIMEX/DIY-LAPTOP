#include<clk/clk.h>

static HLIST_HEAD(clk_list);

struct clk *__clk_lookup(const char *name)
{
	struct clk *root_clk;
	//struct clk *ret;
	struct hlist_node *tmp;
	int test_count = 0;
	if (!name)
		return NULL;

	/* search the 'proper' clk tree first */
	hlist_for_each_entry(root_clk, tmp, &clk_list, child_node) {
		//printf("============%s: node name: %s===========test_count: %d\n",__func__, root_clk->name,test_count);
		test_count++;
		if (!strcmp(root_clk->name, name))
			return root_clk;
	}

	return NULL;
}

static void __clk_disable(struct clk *clk)
{
	if (!clk)
		return;

	if (clk->enable_count == 0)
		return;

	if (--clk->enable_count > 0)
		return;

	if (clk->ops->disable)
		clk->ops->disable(clk->hw);

	__clk_disable(clk->parent);
}

unsigned long __clk_get_rate(struct clk *clk)
{
	unsigned long ret;

	if (!clk) {
		ret = 0;
		goto out;
	}
	ret = clk->rate;

out:
	return ret;
}

static int __clk_enable(struct clk *clk)
{
	int ret = 0;

	/*printf("%s:clk-name: %s, clk-parent-name: %s, clk->flags = %lu.\n",
			__func__,clk->name,clk->parent->name, clk->flags);*/

	if (!clk){

		printf("%s: clk is null.\n",__func__);
		return -1;
	}
	if (clk->flags & CLK_IS_ROOT) {
		//printf("%s: clk is root.\n",__func__);
		return 0;
	}

	if (clk->enable_count == 0) {
		ret = __clk_enable(clk->parent);
		if (ret)
			return ret;

		if (clk->ops->enable) {
			ret = clk->ops->enable(clk->hw);
			if (ret) {
				__clk_disable(clk->parent);
				return ret;
			}
		}
	}

	clk->enable_count++;
	return 0;
}

static int __clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *old_parent;
	int ret = -1;
	u8 i;

	//printf("%s:",__func__);
	old_parent = clk->parent;

	if (!clk->parents) {
		clk->parents = malloc(sizeof(struct clk*) * clk->num_parents);
		if(clk->parents)
			memset(clk->parents, 0, sizeof(struct clk*) * clk->num_parents);
	}

	/*
	 * find index of new parent clock using cached parent ptrs,
	 * or if not yet cached, use string name comparison and cache
	 * them now to avoid future calls to __clk_lookup.
	 */
	for (i = 0; i < clk->num_parents; i++) {
		if (clk->parents && clk->parents[i] == parent) {
			break;
		}
		else if (!strcmp(clk->parent_names[i], parent->name)) {
			if (clk->parents)
				clk->parents[i] = __clk_lookup(parent->name);
			break;
		}
	}

	if (i == clk->num_parents) {
		printf("%s: clock %s is not a possible parent of clock %s\n",
				__func__, parent->name, clk->name);
		goto out;
	}


	/* FIXME replace with clk_is_enabled(clk) someday */
	if (clk->enable_count)
		__clk_enable(parent);

	/* change clock input source */
	ret = clk->ops->set_parent(clk->hw, i);
	clk->parent = clk->parents[i];
	/* clean up old prepare and enable */
	if (clk->enable_count)
		__clk_disable(old_parent);

out:
	return ret;
}

static void __clk_recalc_rates(struct clk *clk)
{
	unsigned long parent_rate = 0;

	//printf("%s:\n",__func__);
	if (clk->parent)
		parent_rate = clk->parent->rate;

	if (clk->ops->recalc_rate)
		clk->rate = clk->ops->recalc_rate(clk->hw, parent_rate);
	else
		clk->rate = parent_rate;
}

static struct clk *__clk_init_parent(struct clk *clk)
{
	struct clk *ret = NULL;
	u8 index;

	/* handle the trivial cases */
	if (!clk->num_parents)
		goto out;
	
	//printf("%s: clk_name :%s\n",__func__,clk->name);
	if (clk->num_parents == 1) {
		if (!clk->parent)
			ret = clk->parent = __clk_lookup(clk->parent_names[0]);
		ret = clk->parent;
		goto out;
	}

	if (!clk->ops->get_parent) {
		printf("%s: multi-parent clocks must implement .get_parent\n",
			__func__);
		goto out;
	};

	/*
	 * Do our best to cache parent clocks in clk->parents.  This prevents
	 * unnecessary and expensive calls to __clk_lookup.  We don't set
	 * clk->parent here; that is done by the calling function
	 */

	index = clk->ops->get_parent(clk->hw);

	if (!clk->parents) {
		clk->parents =
			malloc(sizeof(struct clk*) * clk->num_parents);
		if(clk->parents)
			memset(clk->parents, 0, sizeof(struct clk*) * clk->num_parents);
	}
	if (!clk->parents)
		ret = __clk_lookup(clk->parent_names[index]);
	else if (!clk->parents[index])
		ret = clk->parents[index] =
			__clk_lookup(clk->parent_names[index]);
	else
		ret = clk->parents[index];

out:
	return ret;
}


int __clk_init(struct clk *clk)
{
	int i, ret = 0;
	//struct clk *orphan;
	//struct hlist_node *tmp, *tmp2;

	if (!clk) {
		printf("%s: this is a null clk!\n",__func__);
		return -1;
	}
	/* check to see if a clock with this name is already registered */
	if (__clk_lookup(clk->name)) {
		printf("%s: clk %s already initialized\n",
				__func__, clk->name);
		ret = -1;
		goto out;
	}
	/* check that clk_ops are sane.  See Documentation/clk.txt */
	if (clk->ops->set_rate &&
			!(clk->ops->round_rate && clk->ops->recalc_rate)) {
		printf("%s: %s must implement .round_rate & .recalc_rate\n",
				__func__, clk->name);
		ret = -1;
		goto out;
	}

	if (clk->ops->set_parent && !clk->ops->get_parent) {
		printf("%s: %s must implement .get_parent & .set_parent\n",
				__func__, clk->name);
		ret = -1;
		goto out;
	}

	/* throw a WARN if any entries in parent_names are NULL */
	for (i = 0; i < clk->num_parents; i++)
		if(!clk->parent_names[i])
		printf("%s: invalid NULL in %s's .parent_names\n",
				__func__, clk->name);
	/*
	 * Allocate an array of struct clk *'s to avoid unnecessary string
	 * look-ups of clk's possible parents.  This can fail for clocks passed
	 * in to clk_init during early boot; thus any access to clk->parents[]
	 * must always check for a NULL pointer and try to populate it if
	 * necessary.
	 *
	 * If clk->parents is not NULL we skip this entire block.  This allows
	 * for clock drivers to statically initialize clk->parents.
	 */

	if (clk->num_parents > 1 && !clk->parents) {
		clk->parents = malloc((sizeof(struct clk*) * clk->num_parents));
		if(clk->parents)
			memset(clk->parents, 0, sizeof(struct clk*) * clk->num_parents);
		/*
		 * __clk_lookup returns NULL for parents that have not been
		 * clk_init'd; thus any access to clk->parents[] must check
		 * for a NULL pointer.  We can always perform lazy lookups for
		 * missing parents later on.
		 */
		if (clk->parents)
			for (i = 0; i < clk->num_parents; i++)
				clk->parents[i] =
					__clk_lookup(clk->parent_names[i]);
	}
	clk->parent = __clk_init_parent(clk);
	/*printf("============%s: clk name: %s, clk_num_parents: %d===========\n",
				__func__, clk->name,clk->num_parents);*/
	/*
	 * optional platform-specific magic
	 *
	 * The .init callback is not used by any of the basic clock types, but
	 * exists for weird hardware that must perform initialization magic.
	 * Please consider other ways of solving initialization problems before
	 * using this callback, as it's use is discouraged.
	 */


	hlist_add_head(&clk->child_node,
				&clk_list);

	if (clk->ops->recalc_rate)
		clk->rate = clk->ops->recalc_rate(clk->hw,
				__clk_get_rate(clk->parent));
	else if (clk->parent)
		clk->rate = clk->parent->rate;
	else
		clk->rate = 0;
	
	if (clk->ops->init)
		clk->ops->init(clk->hw);
out:

	return ret;
}


int clk_disable(struct clk *clk)
{
	if (!clk)
		return -1;

	 __clk_disable(clk);

	 return 0;
}


int clk_prepare_enable(struct clk *clk)
{
	int ret;
	if (!clk)
		return -1;

	ret = __clk_enable(clk);

	return ret;
}

struct clk *clk_get_parent(struct clk *clk)
{
	return !clk ? NULL : clk->parent;
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = 0;
	//printf("%s:\n",__func__);
	if (!clk || !clk->ops)
		return -1;

	if (!clk->ops->set_parent)
		return -1;

	/* prevent racing with updates to the clock topology */

	if (clk->parent == parent)
		goto out;

	/* only re-parent if the clock is not in use */
	if ((clk->flags & CLK_SET_PARENT_GATE) && clk->prepare_count) {

		printf("%s: clk->enable_count = %d\n",__func__, clk->enable_count);
		ret = -1;
	}
	else
		ret = __clk_set_parent(clk, parent);

	/* propagate ABORT_RATE_CHANGE if .set_parent failed */
	if (ret) {
		__clk_recalc_rates(clk);
		goto out;
	}

out:

	return ret;
}

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long rate;

	//printf("%s:\n",__func__);
	rate = __clk_get_rate(clk);

	return rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long best_parent_rate = 0;
	int ret = 0;
	unsigned long new_rate;

	//printf("%s:\n",__func__);
	/* bail early if nothing to do */
	if (rate == clk->rate && !(clk->flags & CLK_GET_RATE_NOCACHE)) {
		printf("%s: no need to set rate again.\n", __func__);
		goto out;
	}

	/* calculate new rates and get the topmost changed clock */
	if (clk->parent)
		best_parent_rate = clk->parent->rate;

	if (!clk->parent) {
		printf("%s: %s has NULL parent\n", __func__, clk->name);
		ret = -1;

		goto out;
	}

	if (!clk->ops->round_rate)
		new_rate = clk->parent->new_rate;
	else {
		new_rate = clk->ops->round_rate(clk->hw, rate, &best_parent_rate);
		//printf("%s: switch round_rate. new_rate = %lu.\n", __func__,new_rate);
	}
	clk->new_rate = new_rate;


	/* change the rates */
	if (clk->ops->set_rate)
		clk->ops->set_rate(clk->hw, clk->new_rate, best_parent_rate);

	if (clk->ops->recalc_rate)
		clk->rate = clk->ops->recalc_rate(clk->hw, best_parent_rate);
	else
		clk->rate = best_parent_rate;

	//printf("%s: new_rate = %lu, clk_new_rate = %lu, clk_rate = %lu.\n",
				//__func__, new_rate, clk->new_rate, clk->rate);
	return 0;
out:

	return ret;
}



struct clk *clk_get(void *dev, const char *con_id)
{
	struct clk *clk;
	clk = __clk_lookup(con_id);

	return clk ? clk : NULL;
}

void clk_put(struct clk *clk)
{
}

void  clk_init(void)
{
	init_clocks();
}

struct clk *clk_register(struct clk_hw *hw)
{
	int ret;
	struct clk *clk;

	clk = malloc(sizeof(*clk));
	if(clk)
		memset(clk, 0, sizeof(*clk));
	if (!clk) {
		printf("%s: could not allocate clk\n", __func__);
		ret = -1;
		goto fail_out;
	}
	clk->name = hw->init->name;
	clk->ops = hw->init->ops;
	clk->hw = hw;
	clk->flags = hw->init->flags;
	clk->parent_names = hw->init->parent_names;
	clk->num_parents = hw->init->num_parents;
	hw->clk = clk;

	ret = __clk_init(clk);
	if (!ret)
        return clk;

	free(clk);
fail_out:
	return NULL;
}




